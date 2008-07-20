#include "extractdecodethread.h"

#include <QTime>
#include <QProcess>
#include <QImageReader>
#include <KDebug>

#include "page.h"

ExtractDecodeThread::ExtractDecodeThread(FileClassifier::ArchiveType archiveType, const QString &archive, QObject *parent)
 : DecodeThread(parent)
{
    // Choose the right tool and options
    switch (archiveType) {
        case FileClassifier::Tar:
            _command = "tar";
            _args<<"-xOf";
            break;
        case FileClassifier::TarGz:
            _command = "tar";
            _args<<"-zxOf";
            break;
        case FileClassifier::TarBz:
            _command = "tar";
            _args<<"--bzip2"<<"-xOf";
            break;
        case FileClassifier::TarZ:
            _command = "tar";
            _args<<"-ZxOf";
            break;
        case FileClassifier::Zip:
            _command = "unzip";
            _args<<"-p";
            break;
        case FileClassifier::Rar:
            _command = "unrar";
            _args<<"p"<<"-ierr";
            // Note: With "-ierr", the header info is put into stderr (and ignored here)
            break;
        default:
            Q_ASSERT(false);
    }
    // Pass in the file name
    _args<<archive;

    kDebug()<<_command<<_args;
}


ExtractDecodeThread::~ExtractDecodeThread()
{
}

void ExtractDecodeThread::decode()
{
    QTime time;

    QProcess extracter;

    //kDebug()<<"start"<<_page->getPageNumber();
    time.start();

    // Start the extracter
    extracter.start(_command, _args + QStringList(_page->getFilename()));

    // Wait for the extracter to finish
    // Note: QImageReader won't decode the whole image unless the whole
    //  image is ready to be read
    bool waited = extracter.waitForFinished();
    Q_ASSERT(waited);

    // If aborted, discard the decode

    // Set up the image reader
    QImageReader imageReader(&extracter);


    // If the full size is known, calculate the prescaled size
    bool prescaled = _page->isFullImageSizeKnown();

    if (prescaled) {

        // Notify the page that the decode size is set
        QSize targetSize = _page->getTargetSize();
        _page->decodeSizeSet(targetSize);

        // Tell the image reader to output prescaled
        imageReader.setScaledSize(targetSize);
    }

    // Decode the image
    QImage image(imageReader.read());
    Q_ASSERT(!image.isNull());

    if (!prescaled) {
        // The image was decoded at the full size because we "can't" tell the size
        //  before starting decoding

        // Retrieve the size
        QSize fullSize = image.size();
        Q_ASSERT(fullSize.isValid());

        // Save the size
        _page->setFullImageSize(fullSize);

        // Notify the page that the decode size is set
        QSize targetSize = _page->getTargetSize();
        _page->decodeSizeSet(targetSize);

        // Resize the image now
        image = image.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Create the thumbnail
    QImage thumbnail = image.scaled(Page::THUMBNAIL_SIZE, Qt::KeepAspectRatio);

    //kDebug()<<"done"<<_page->getPageNumber()<<":"<<time.elapsed()<<"ms";

    // Decode job now finished
    finished(image, thumbnail);
}

#include "extractdecodethread.moc"
