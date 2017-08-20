#include "decoder.h"

#include <QFileInfo>
#include <QProcess>
#include <QTextCodec>
#include <QtConcurrentRun>

#include "archive.h"
#include "debug.h"
#include "fileclassification.h"
#include "imagesource.h"
#include "indexer.h"
#include "strategist.h"

Decoder::Decoder(QObject *parent)
    : QObject(parent)
{
    _cancelled = false;
}

Decoder::~Decoder()
{
    // Cancel any decode
    cancel();

    // Wait for the decode to finish
    _measureFuture.waitForFinished();
    _decodeFuture.waitForFinished();

    delete _imageSource;
    delete _extracter;
    //debug()<<"~Decoder()";
}

int Decoder::pageNum()
{
    return _pageNum;
}

void Decoder::cancel()
{
    _cancelled = true;

    debug()<<"Cancelling"<<_pageNum;

    // Prevent image decoding
    _imageSource->close();

    // Stop the extracter process
    //debug()<<"Terminating extracter process after"<<_time.elapsed()<<"ms";
    _extracter->terminate();

    if (_extracter->state() != QProcess::NotRunning)
    {
        // Wait kindly for it to finish
        _extracter->waitForFinished(KILL_WAIT);

        if (_extracter->state() != QProcess::NotRunning)
        {
            // Kill the process if it's still running
            debug()<<"Killing extracter process";
            _extracter->kill();
        }
    }
}

/**
 * @todo Use correct path on Windows to find 7z
 */
QStringList Decoder::chooseExtracterArguments(
    const Archive &archive,
    const QByteArray &pageFilename)
{
    QStringList args;

    switch (archive.type())
    {
        case Archive::SevenZip:
            args<<"e"<<"-so";
#ifdef Q_OS_WIN32
            // Specify the list file encoding on Windows
            args<<"-scsDOS";
#endif
            break;
        case Archive::Tar:
            args<<"-xOf";
            break;
        case Archive::Zip:
            args<<"-p";
            break;
        case Archive::Rar:
            args<<"p"<<"-ierr";
            // Note: With "-ierr", the header info is put into stderr (and not into the image data)
            break;
        default:
            Q_ASSERT(false);
    }

    // Pass in the archive file name
    args<<archive.filename();

    // Pass in the name of the compressed file
    if (archive.type() == Archive::SevenZip)
    {
        _temporaryFile.open();
        _temporaryFile.write(pageFilename);
        _temporaryFile.write("\n");
        _temporaryFile.flush();

        args<<("-i@" + _temporaryFile.fileName());
    }
    else
    {
        args<<QString::fromLocal8Bit(pageFilename);
    }

    return args;
}

void Decoder::startExtracter(
    const Archive &archive,
    const QByteArray &pageFilename)
{
    QString command = archive.programPath();
    QStringList args = chooseExtracterArguments(archive, pageFilename);
    _extracter = new QProcess();
    _extracter->start(command, args);
    //debug()<<"Starting"<<command<<args;
}

void Decoder::makeImageSource(int uncompressedSize)
{
    // Set up blocking-IO cancellable proxy
    _imageSource = new ImageSource(_extracter, uncompressedSize);
}

void Decoder::setUpImageReader(const QByteArray &pageFilename)
{
    _imageReader.setDevice(_imageSource);

    _imageReader.setFormat(
        QFileInfo(pageFilename)
            .suffix()
            .toLower()
            .toLatin1());

    if (!_strategist->isFullPageSizeKnown(_pageNum))
    {
        startMeasuring();
    }
    else
    {
        startDecoding();
    }
}

void Decoder::startMeasuring()
{
    // Create the future watcher
    QFutureWatcher<QSize> *measureWatcher = new QFutureWatcher<QSize>(this);
    connect(measureWatcher, SIGNAL(finished()), SLOT(measureFinished()));

    // Start the future
    _measureFuture = QtConcurrent::run(
        &_imageReader, &QImageReader::size);

    // Subscribe to the future finishing
    measureWatcher->setFuture(_measureFuture);
}

void Decoder::measureFinished()
{
    if (!_cancelled)
    {
        // Retrieve the full image size
        QSize fullSize = _measureFuture.result();
        Q_ASSERT(fullSize.isValid());
        debug()<<"Found     "<<_pageNum<<fullSize;

        // Save it
        _strategist->setFullPageSize(_pageNum, fullSize);

        // Start the decode
        startDecoding();
    }
    else
    {
        // Cancelled
        emit cancelled(this);
    }
}

void Decoder::startDecoding()
{
    // Set up scaling
    QSize layout = _strategist->pageLayout(_pageNum).size();
    _imageReader.setScaledSize(layout);
    debug()<<"Layout    "<<_pageNum<<layout;

    // Create the future watcher
    QFutureWatcher<QImage> *decodeWatcher = new QFutureWatcher<QImage>(this);
    connect(decodeWatcher, SIGNAL(finished()), SLOT(decodeFinished()));

    // Start the future
    _decodeFuture = QtConcurrent::run(
        &_imageReader, &QImageReader::read);

    // Subscribe to the future finishing
    decodeWatcher->setFuture(_decodeFuture);
}

void Decoder::decodeFinished()
{
    //debug()<<"Decoded"<<_pageNum<<"--"<<_time.elapsed()<<"ms";

    // Give notification
    if (!_cancelled)
    {
        // Done, assert the decode was okay
        QImage image = _decodeFuture.result();
        Q_ASSERT(!image.isNull());

        // Conver the image to a pixmap
        QPixmap pixmap(QPixmap::fromImage(image));

        emit done(this, _pageNum, pixmap);
    }
    else
    {
        // Cancelled
        emit cancelled(this);
    }
}

void Decoder::decode(
    const Archive &archive,
    const Indexer &indexer,
    Strategist &strategist,
    int pageNum)
{
    _strategist = &strategist;
    _pageNum = pageNum;
    _time.start();

    QByteArray pageFilename = indexer.pageName(_pageNum); 
    int uncompressedSize = indexer.uncompressedSize(_pageNum);

    startExtracter(archive, pageFilename);

    makeImageSource(uncompressedSize);

    setUpImageReader(pageFilename);
}
