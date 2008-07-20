#include <KDebug>
#include <QTime>
#include <QImageReader>
#include <QProcess>
#include <QImage>

#include "extractdecodethread.h"
#include "pagecache.h"

ExtractDecodeThread::ExtractDecodeThread(PageCache *pageCache)
    :DecodeThread(pageCache)
{
}
ExtractDecodeThread:: ~ExtractDecodeThread()
{
}

void ExtractDecodeThread::setArchive(FileInfo::ArchiveType archiveType, const QString &archive)
{
    kDebug()<<"Setting archive "<<archive<<" (type="<<archiveType<<")"<<endl;
    _archiveType = archiveType;
    _archive = archive;
}

void ExtractDecodeThread::decode(int pageNum, const QString &path, QSize fullSize, const QSize &boundingSize)
{
    QTime time;

    QProcess extracter;

    //kDebug()<<"Extracting and decoding "<<path()<<endl;
    time.start();

    QString command;
    QStringList args;

    switch (_archiveType) {
    case FileInfo::Tar:
        command = "tar";
        args<<"-xOf";
        break;
    case FileInfo::TarGz:
        command = "tar";
        args<<"-zxOf";
        break;
    case FileInfo::TarBz:
        command = "tar";
        args<<"--bzip2"<<"-xOf";
        break;
    case FileInfo::TarZ:
        command = "tar";
        args<<"-ZxOf";
        break;
    case FileInfo::Zip:
        command = "unzip";
        args<<"-p";
        break;
    case FileInfo::Rar:
        command = "unrar";
        args<<"p"<<"-ierr";
        // Note: With "-ierr", the header info is put into stderr (and ignored here)
        break;
    default:
        Q_ASSERT(false);
    }
    args<<_archive<<path;

    extracter.start(command, args);

    // Wait for the extracter to finish
    // Note: QImageReader won't decode the whole image unless the whole
    //  image is ready to be read
    bool waited = extracter.waitForFinished();
    Q_ASSERT(waited);

    // If aborted, discard the decode
    if (decodeAborted()) {
        return;
    }

    // Set up the image reader
    QImageReader imageReader(&extracter);

    // If the full size is known, scale the image as reading it
    bool prescaled = fullSize.isValid();
    if (prescaled && boundingSize.isValid()) {
        QSize imageSize(fullSize);

        // Scale the image down
        imageSize.scale(boundingSize, Qt::KeepAspectRatio);
        imageReader.setScaledSize(imageSize);
    }

    // Have the image reader start
    QImage image(imageReader.read());

    // The image is decoded at the full size because we "can't" tell the size
    //  before starting decoding
    fullSize = image.size();
    Q_ASSERT(fullSize.isValid());

    // So scale the decoded image in the worker thread
    if (!prescaled && boundingSize.isValid()) {
        // Scale the image down
        // Note: Fast transform looks horrible
        image = image.scaled(boundingSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    //kDebug()<<"Extracting and decoding finished: "<<time.elapsed()<<" ms"<<endl;

    // If aborted, discard the decode
    if (decodeAborted()) {
        return;
    }

    // Cache the decode
    pageCache()->setData(pageNum, image, fullSize);
}

#include "extractdecodethread.moc"
