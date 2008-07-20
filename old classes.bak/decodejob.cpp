#include <KDebug>

#include "decodejob.h"

DecodeJob::DecodeJob(int pageNum, const QString &path, const QSize &boundingSize, bool highPriority)
:_pageNum(pageNum), _path(path), _boundingSize(boundingSize), _highPriority(highPriority)
{
}

DecodeJob::~DecodeJob()
{
    //kDebug()<<"DecodeJob deconstructing"<<endl;
}

int DecodeJob::priority() const
{
    return _highPriority ?1 :0;
}

void DecodeJob::resetPriority(bool highPriority)
{
    _highPriority = highPriority;
}

void DecodeJob::resetBoundingSize(const QSize &boundingSize)
{
    _boundingSize = boundingSize;
}

int DecodeJob::pageNum() const
{
    return _pageNum;
}
QImage &DecodeJob::image()
{
    // Note: http://lists.trolltech.com/qt4-preview-feedback/2005-09/msg00037.html
    //  QPixmap is not threadsafe because accesses to xlib aren't
    //  QImage is OK to use in a non GUI-thread
    return _image;
}
const QSize &DecodeJob::fullImageSize() const
{
    return _fullImageSize;
}
const QString &DecodeJob::path() const
{
    return _path;
}
const QSize &DecodeJob::boundingSize() const
{
    return _boundingSize;
}
void DecodeJob::setImage(const QImage &image)
{
    _image = image;
}
void DecodeJob::setFullImageSize(const QSize &size)
{
    _fullImageSize = size;
}

#include "decodejob.moc"
