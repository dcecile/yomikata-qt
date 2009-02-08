#include "decodethread.h"

#include <QTime>
#include <QProcess>
#include <QImageReader>

#include "debug.h"
#include "indexer.h"
#include "strategist.h"
#include "fileclassification.h"

DecodeThread::DecodeThread(const Indexer &indexer, Strategist &strategist, QObject *parent)
    : QThread(parent), _indexer(indexer), _strategist(strategist)
{
    _pageNum = -1;
    _requestPageNum = -1;
    _aborted = false;
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
    QMutexLocker locker(&_requestLock);

    // Assert no current decode
    Q_ASSERT(_pageNum == -1);

    // Create a request
    _requestPageNum = index;
    _decodeRequest.wakeAll();
}

int DecodeThread::currentPageNum()
{
    QMutexLocker locker(&_requestLock);
    return _pageNum;
}

void DecodeThread::cancel()
{
    // Set the cancelled flag
    _cancelled = true;
}

void DecodeThread::run()
{
    while (true)
    {
        // No current page
        _requestLock.lock();
        _pageNum = -1;

        // Check if aborted during last decode
        if (_aborted)
        {
            return;
        }

        // Check for a queued request
        if (_requestPageNum == -1)
        {
            // Wait for a request
            _decodeRequest.wait(&_requestLock);
        }

        // Check if aborted while waiting
        if (_aborted)
        {
            return;
        }

        // Check if reset
        if (_reset)
        {
            setExtractCommand();
            _reset = false;
        }

        // Set the page number
        _pageNum = _requestPageNum;
        _requestPageNum = -1;

        // Done getting request parameters
        _requestLock.unlock();

        // Not cancelled
        _cancelled = false;

        // Start the decode
        decode();
    }
}

void DecodeThread::setExtractCommand()
{
    QString archive = _indexer.filename();
    FileClassification::ArchiveType archiveType = FileClassification::getArchiveType(archive);

    // Choose the right tool and options
    _args.clear();

    switch (archiveType)
    {
        case FileClassification::Tar:
            _command = "tar";
            _args<<"-xOf";
            break;
        case FileClassification::TarGz:
            _command = "tar";
            _args<<"-zxOf";
            break;
        case FileClassification::TarBz:
            _command = "tar";
            _args<<"--bzip2"<<"-xOf";
            break;
        case FileClassification::TarZ:
            _command = "tar";
            _args<<"-ZxOf";
            break;
        case FileClassification::Zip:
            _command = "unzip";
            _args<<"-p";
            break;
        case FileClassification::Rar:
            _command = "unrar";
            _args<<"p"<<"-ierr";
            // Note: With "-ierr", the header info is put into stderr (and ignored here)
            break;
        default:
            Q_ASSERT(false);
    }

    // Pass in the file name
    _args<<archive;

    debug()<<"Extract command set:"<<_command<<_args;
}

/**
 * @todo Really cancel the extracter if this decode isn't needed.
 */
void DecodeThread::decode()
{
    QTime time;

    QProcess extracter;

    //debug()<<"start"<<_page->getPageNumber();
    time.start();

    // Start the extracter
    extracter.start(_command, _args + QStringList(_indexer.pageName(_pageNum)));

    // Wait for the extracter to finish
    // Note: QImageReader won't decode the whole image unless the whole
    //  image is ready to be read

    // Keep waiting until done or cancelled
    bool waited;

    do
    {
        waited = extracter.waitForFinished(CANCELLED_POLLING);
    }
    while (!_cancelled && !waited);

    // Stop the extracter if cancelled
    if (_cancelled)
    {
        // Send a SIGTERM signal
        debug()<<"Terminating extracter process";
        extracter.terminate();

        // Wait kindly for it to finish
        bool finished = extracter.waitForFinished(KILL_WAIT);

        // Kill the process if it's still running
        if (!finished)
        {
            debug()<<"Killing extracter process";
            extracter.kill();
        }

        // Notify cancelled
        prepareForRequest();
        emit cancelled(this);
        return;
    }

    // Set up the image reader
    QImageReader imageReader(&extracter);

    // If the full size is known, calculate the prescaled size
    bool prescaled = _strategist.isFullPageSizeKnown(_pageNum);

    if (prescaled)
    {
        // Notify the page that the decode size is set
        QSize targetSize = _strategist.pageLayout(_pageNum).size();

        // Tell the image reader to output prescaled
        imageReader.setScaledSize(targetSize);
    }

    // Decode the image
    QImage image(imageReader.read());
    Q_ASSERT(!image.isNull());

    if (!prescaled)
    {
        // The image was decoded at the full size because we "can't" tell the size
        //  before starting decoding

        // Retrieve the size
        QSize fullSize = image.size();
        Q_ASSERT(fullSize.isValid());

        // Save the size
        _strategist.setFullPageSize(_pageNum, fullSize);

        // Check if cancelled, after storing the full size
        if (_cancelled)
        {
            prepareForRequest();
            emit cancelled(this);
            return;
        }

        // Resize the image now
        QSize targetSize = _strategist.pageLayout(_pageNum).size();
        image = image.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    debug()<<"Decoded"<<_pageNum<<"--"<<time.elapsed()<<"ms";

    // Decode job now finished
    int pageNum = _pageNum;
    prepareForRequest();
    emit done(this, pageNum, image);
}

void DecodeThread::prepareForRequest()
{
    QMutexLocker locker(&_requestLock);
    _pageNum = -1;
}

#include "decodethread.moc"
