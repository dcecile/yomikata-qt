#include <KDebug>
#include <QTime>
#include <QImageReader>

#include "filedecodejob.h"

FileDecodeJob::FileDecodeJob(int pageNum, const QString &path, const QSize &boundingSize)
    :DecodeJob(pageNum, path, boundingSize)
{
}

void FileDecodeJob::run()
{
    QTime time;

    QImageReader imageReader(path());

    setFullImageSize(imageReader.size());
    Q_ASSERT(fullImageSize().isValid());

    if (boundingSize().isValid()) {
        QSize imageSize(fullImageSize());

        // Scale the image down
        imageSize.scale(boundingSize(), Qt::KeepAspectRatio);
        imageReader.setScaledSize(imageSize);
    }

    kDebug()<<"Decoding "<<path()<<endl;
    time.start();
    setImage(imageReader.read());
    kDebug()<<"Decoding finished: "<<time.elapsed()<<" ms"<<endl;
}

#include "filedecodejob.moc"
