#include <KDebug>
#include <QTime>
#include <QImageReader>

#include "filedecodethread.h"
#include "pagecache.h"

FileDecodeThread::FileDecodeThread(PageCache *pageCache)
    :DecodeThread(pageCache)
{
}
FileDecodeThread:: ~FileDecodeThread()
{
}

void FileDecodeThread::decode(int pageNum, const QString &path, QSize fullSize, const QSize &boundingSize)
{
    QTime time;

    // Init the image reader
    QImageReader imageReader(path);

    // See if the full size is known
    if (!fullSize.isValid()) {
        // Retrieve the full size
        fullSize = imageReader.size();
        Q_ASSERT(fullSize.isValid());
    }

    // Check the bounding size
    // If there's a bounding size, scale the image
    if (boundingSize.isValid()) {
        QSize imageSize(fullSize);

        // Scale the image down
        imageSize.scale(boundingSize, Qt::KeepAspectRatio);
        imageReader.setScaledSize(imageSize);
    }

    //kDebug()<<"Decoding "<<path()<<endl;
    time.start();

    // Decode the image
    QImage image(imageReader.read());

    // If aborted, discard the decode
    if (decodeAborted()) {
        return;
    }

    // Cache the decode
    pageCache()->setData(pageNum, image, fullSize);
}

#include "filedecodethread.moc"
