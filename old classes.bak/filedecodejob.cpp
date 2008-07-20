#include <KDebug>
#include <QTime>
#include <QImageReader>

#include "filedecodejob.h"

FileDecodeJob::FileDecodeJob(int pageNum, const QString &path, const QSize &boundingSize, bool highPriority)
    :DecodeJob(pageNum, path, boundingSize, highPriority)
{
}

void FileDecodeJob::run()
{
    //kDebug()<<"Decoding finished: "<<time.elapsed()<<" ms"<<endl;
}

#include "filedecodejob.moc"
