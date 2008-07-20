#include <KDebug>
#include <QTime>
#include <QImageReader>
#include <QProcess>

#include "extractdecodejob.h"

ExtractDecodeJob::ExtractDecodeJob(int pageNum, const QString &path, const QSize &boundingSize, FileInfo::ArchiveType archiveType, const QString &archive)
    :DecodeJob(pageNum, path, boundingSize), _archiveType(archiveType), _archive(archive)
{
}

void ExtractDecodeJob::run()
{
    QTime time;

    QProcess extracter;

    kDebug()<<"Extracting and decoding "<<path()<<endl;
    time.start();

    QString command;
    QStringList args;

    switch (_archiveType) {
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
    args<<_archive<<path();

    extracter.start(command, args);

    // Wait for the extracter to finish
    // Note: QImageReader won't decode the whole image unless the whole
    //  image is ready to be read
    bool waited = extracter.waitForFinished();
    Q_ASSERT(waited);

    // Have the image reader start
    QImageReader imageReader(&extracter);
    setImage(imageReader.read());

    // The image is decoded at the full size because we "can't" tell the size
    //  before starting decoding
    setFullImageSize(image().size());
    Q_ASSERT(fullImageSize().isValid());

    // So scale the decoded image in the worker thread
    if (boundingSize().isValid()) {
        // Scale the image down
        // Note: Fast transform looks horrible
        setImage(image().scaled(boundingSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    kDebug()<<"Extracting and decoding finished: "<<time.elapsed()<<" ms"<<endl;
}

#include "extractdecodejob.moc"
