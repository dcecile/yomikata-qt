#include "decodethread.h"

#include <QTime>
#include <QProcess>
#include <QImageReader>
#include <QTextCodec>
#include <QTemporaryFile>

#include "debug.h"
#include "archive.h"
#include "indexer.h"
#include "strategist.h"
#include "fileclassification.h"
#include "imagesource.h"

DecodeThread::DecodeThread(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent)
    : QThread(parent), _archive(archive), _indexer(indexer), _strategist(strategist)
{
    _pageNum = -1;
    _requestPageNum = -1;
    _aborted = false;
    _imageSource = NULL;

    QTemporaryFile file;
    file.setAutoRemove(false);
    file.open();
    _temporaryFileName = file.fileName();
}

DecodeThread::~DecodeThread()
{
    // Cancel any decode
    cancel();

    // Create an abort request
    _requestLock.lock();
    _aborted = true;
    _decodeRequest.wakeAll();
    _requestLock.unlock();

    // Wait for the thread to finish
    wait();

    // Remove the temporary file
    QFile(_temporaryFileName).remove();

    debug()<<"~DecodeThread()";
}

void DecodeThread::reset()
{
    // Cancel any decode
    cancel();

    // Make the next request reset
    QMutexLocker locker(&_requestLock);
    _reset = true;
}

/**
 * @todo Find out how to prevent queuing
 */
void DecodeThread::decode(int index)
{
    if (_requestLock.tryLock())
    {
        _requestLock.unlock();
    }
    else
    {
        debug()<<"decode blocked";
    }

    QMutexLocker locker(&_requestLock);

    // Assert no current decode
    Q_ASSERT(_pageNum == -1);

    // Create a request
    _requestPageNum = index;
    _decodeRequest.wakeAll();
}

int DecodeThread::currentPageNum()
{
    if (_requestLock.tryLock())
    {
        _requestLock.unlock();
    }
    else
    {
        debug()<<"currentPageNum blocked";
    }

    QMutexLocker locker(&_requestLock);
    return _pageNum;
}

void DecodeThread::cancel()
{
    if (_cancelLock.tryLock())
    {
        _cancelLock.unlock();
    }
    else
    {
        debug()<<"cancel blocked";
    }

    QMutexLocker locker(&_cancelLock);

    // Set the cancelled flag
    _cancelled = true;

    // Stop the image decode if it's running
    if (_imageSource != NULL)
    {
        _imageSource->close();
    }
}

void DecodeThread::run()
{
    // Available for requests, no current page
    _pageNum = -1;

    while (true)
    {
        // Get the next request
        {
            QMutexLocker locker(&_requestLock);

            // Check if aborted during last decode
            if (_aborted)
            {
                return;
            }

            // Wait if no queued request
            if (_requestPageNum == -1)
            {
                _decodeRequest.wait(&_requestLock);
            }

            // Stop if aborted while waiting
            if (_aborted)
            {
                return;
            }

            // Reload the command if reset
            if (_reset)
            {
                setExtractCommand();
                _reset = false;
            }

            // Set the page number
            _pageNum = _requestPageNum;
            _requestPageNum = -1;

            // Done getting request parameters
        }

        // Not cancelled
        _cancelled = false;

        // Wait to see if cancelled
        QTime clock;
        clock.start();

        for (int i = 0; i < 500 && !_cancelled; i += 10)
        {
            //msleep(10);
        }

        if (!_cancelled)
        {
            // Start the decode
            decode();
        }
        else
        {
            debug()<<"Cancelled before started after"<<clock.elapsed()<<"ms";
        }

        // Available for requests, no current page
        int lastPage = _pageNum;
        {
            QMutexLocker locker(&_requestLock);
            _pageNum = -1;
        }

        // Give notification
        if (!_cancelled)
        {
            // Done
            emit done(this, lastPage, _image);
        }
        else
        {
            // Cancelled
            emit cancelled(this);
        }
    }
}

/**
 * @todo Use correct path on Windows to find 7z
 */
void DecodeThread::setExtractCommand()
{
    // Set the command
    _command = _archive.programPath();

    // Choose the right tool and options
    _args.clear();

    switch (_archive.type())
    {
        case Archive::SevenZip:
            _args<<"e"<<"-so";
#ifdef Q_OS_WIN32
            // Specify the list file encoding on Windows
            _args<<"-scsDOS";
#endif
            break;
        case Archive::Tar:
            _args<<"-xOf";
            break;
        case Archive::Zip:
            _args<<"-p";
            break;
        case Archive::Rar:
            _args<<"p"<<"-ierr";
            // Note: With "-ierr", the header info is put into stderr (and ignored here)
            break;
        default:
            Q_ASSERT(false);
    }

    // Pass in the file name
    _args<<_archive.filename();
}

/**
 * @todo Really cancel the extracter if this decode isn't needed.
 */
void DecodeThread::decode()
{
    //debug()<<"start"<<_page->getPageNumber();

    QTime time;
    time.start();

    // Choose a format for the filename argument
    QString nameArgument;

    if (_archive.type() == Archive::SevenZip)
    {
    	QFile file(_temporaryFileName);
    	file.open(QIODevice::WriteOnly);
        file.write(_indexer.pageName(_pageNum));
        file.write("\n");
        file.close();

        nameArgument = "-i@" + _temporaryFileName;
    }
    else
    {
    	nameArgument = QString::fromLocal8Bit(_indexer.pageName(_pageNum));
    }

    // Start the extracter
    //debug()<<"Starting"<<_command<<_args + QStringList(nameArgument);
    QProcess extracter;
    QStringList args = _args + QStringList(nameArgument);
    extracter.start(_command, args);

    // Wait for the extracter to finish, until done or cancelled
    // Note: QImageReader won't decode the whole image unless the whole
    //  image is ready to be read
    bool waited;

    do
    {
        QTime clock;
        clock.start();
        waited = extracter.waitForFinished(CANCELLED_POLLING);
        //debug()<<"Waited"<<clock.elapsed();
    }
    while (!_cancelled && !waited);

    //debug()<<extracter.readAllStandardError();

    // Stop the extracter if cancelled
    if (_cancelled)
    {
        // Send a SIGTERM signal
        debug()<<"Terminating extracter process after"<<time.elapsed()<<"ms";
        extracter.terminate();

        // Wait kindly for it to finish
        bool finished = extracter.waitForFinished(KILL_WAIT);

        // Kill the process if it's still running
        if (!finished)
        {
            debug()<<"Killing extracter process";
            extracter.kill();
        }

        return;
    }

    // Set up the image reader
    ImageSource source(&extracter);
    QImageReader imageReader(&source);

    // If the full size is known, calculate the prescaled size
    bool prescaled = _strategist.isFullPageSizeKnown(_pageNum);

    if (prescaled)
    {
        // Notify the page that the decode size is set
        QSize targetSize = _strategist.pageLayout(_pageNum).size();

        // Tell the image reader to output prescaled
        imageReader.setScaledSize(targetSize);
    }

    // Prepare to allow source cancels
    {
        QMutexLocker locker(&_cancelLock);
        _imageSource = &source;
    }

    // Decode the image
    QTime clock;
    clock.start();
    _image = imageReader.read();

    // Disallow source cancels
    {
        QMutexLocker locker(&_cancelLock);
        _imageSource = NULL;
    }

    // Stop if cancelled
    if (_cancelled)
    {
        debug()<<"Cancelled decode after"<<time.elapsed()<<"ms";
        return;
    }

    // Assert the decode was okay
    Q_ASSERT(!_image.isNull());
    //debug()<<"Decode"<<clock.elapsed();

    if (!prescaled)
    {
        // The image was decoded at the full size because we "can't" tell the size
        //  before starting decoding

        // Retrieve the size
        QSize fullSize = _image.size();
        Q_ASSERT(fullSize.isValid());

        // Save the size
        _strategist.setFullPageSize(_pageNum, fullSize);

        // Check if cancelled, after storing the full size
        if (_cancelled)
        {
            return;
        }

        // Resize the image now
        QSize targetSize = _strategist.pageLayout(_pageNum).size();
        _image = _image.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    debug()<<"Decoded"<<_pageNum<<"--"<<time.elapsed()<<"ms";
}

#include "decodethread.moc"
