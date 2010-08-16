#include "decodethread.h"

#include <QFileInfo>
#include <QProcess>
#include <QImageReader>
#include <QTextCodec>
#include <QTemporaryFile>
#include <QtConcurrentRun>

#include "debug.h"
#include "archive.h"
#include "indexer.h"
#include "strategist.h"
#include "fileclassification.h"
#include "imagesource.h"

DecodeThread::DecodeThread(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent)
    : QObject(parent), _archive(archive), _indexer(indexer), _strategist(strategist)
{
    _pageNum = -1;
    _requestPageNum = -1;
    _aborted = false;
    _imageSource = NULL;
    _extracter = NULL;

    QTemporaryFile file;
    file.setAutoRemove(false);
    file.open();
    _temporaryFileName = file.fileName();

    connect(&_decodeWatcher, SIGNAL(finished()), SLOT(decodeFinished()));
}

DecodeThread::~DecodeThread()
{
    // Cancel any decode
    cancel();

    // Remove the temporary file
    QFile(_temporaryFileName).remove();

    debug()<<"~DecodeThread()";
}

void DecodeThread::reset()
{
    // Cancel any decode
    cancel();
}

int DecodeThread::currentPageNum()
{
    return _pageNum;
}

void DecodeThread::cancel()
{
    // Stop the image decode if it's running
    _cancelled = true;

    if (_imageSource != NULL)
    {
        _imageSource->close();
    }
}

void DecodeThread::decodeFinished()
{
    debug()<<"Decoded"<<_pageNum<<"--"<<_time.elapsed()<<"ms";

    Q_ASSERT(_pageNum != -1);
    int finishedPage = _pageNum;
    _pageNum = -1;

    Q_ASSERT(_imageSource != NULL);
    delete _imageSource;
    _imageSource = NULL;

    Q_ASSERT(_extracter != NULL);
    delete _extracter;
    _extracter = NULL;

    // Give notification
    if (!_cancelled)
    {
        // Done, assert the decode was okay
        QImage image = _decodeFuture.result();
        Q_ASSERT(!image.isNull());
        emit done(this, finishedPage, image);
    }
    else
    {
        // Cancelled
        emit cancelled(this);
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
void DecodeThread::decode(int index)
{
    _cancelled = false;

    Q_ASSERT(_pageNum == -1);
    _pageNum = index;
    debug()<<"Decoding"<<_pageNum;

    Q_ASSERT(_imageSource == NULL);
    Q_ASSERT(_extracter == NULL);

    setExtractCommand();

    _time.start();

    // Choose a format for the filename argument
    QByteArray pageName = _indexer.pageName(_pageNum);
    QString nameArgument;

    if (_archive.type() == Archive::SevenZip)
    {
        QFile file(_temporaryFileName);
        file.open(QIODevice::WriteOnly);
        file.write(pageName);
        file.write("\n");
        file.close();

        nameArgument = "-i@" + _temporaryFileName;
    }
    else
    {
        nameArgument = QString::fromLocal8Bit(pageName);
    }

    // Start the extracter
    _extracter = new QProcess;
    QStringList args = _args + QStringList(nameArgument);
    _extracter->start(_command, args);
    //debug()<<"Starting"<<_command<<_args + QStringList(nameArgument);

    // Set up blocking-IO cancellable proxy
    _extracter->waitForFinished();
    _imageSource = new ImageSource(_extracter);

    // Set up the image reader
    _imageReader.setDevice(_imageSource);
    _imageReader.setFormat(
        QFileInfo(pageName).suffix().toLower().toLatin1());

    if (!_strategist.isFullPageSizeKnown(_pageNum))
    {
        // Retrieve the size
        QSize fullSize = _imageReader.size();
        debug()<<fullSize;
        Q_ASSERT(fullSize.isValid());

        // Save the size
        _strategist.setFullPageSize(_pageNum, fullSize);
    }

    // Set up scaling
    _imageReader.setScaledSize(
        _strategist.pageLayout(_pageNum).size());

    // Start decoding the image
    _decodeFuture = QtConcurrent::run(
        &_imageReader, &QImageReader::read);
    _decodeWatcher.setFuture(_decodeFuture);
}

#include "decodethread.moc"
