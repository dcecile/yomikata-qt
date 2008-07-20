#include "page.h"

#include <KDebug>

#include "planner.h"
#include "cache.h"

const QSize Page::THUMBNAIL_SIZE(40, 25);

Page::Page(int pageNumber, const FileInfo &fileInfo, Planner &planner, Cache &cache, QObject *parent)
    : QObject(parent), _planner(planner), _cache(cache), _pageNumber(pageNumber), _filename(fileInfo.name), _filesize(fileInfo.size)
{
}
Page::~Page()
{
}

int Page::getPageNumber() const
{
    return _pageNumber;
}
const QString &Page::getFilename() const
{
    return _filename;
}

bool Page::isFullImageSizeKnown()
{
    return _planner.isFullSizeKnown(_pageNumber);
}
void Page::setFullImageSize(QSize size)
{
    _planner.setFullSize(_pageNumber, size);
}
QSize Page::getTargetSize()
{
    return _planner.plan(_pageNumber).size();
}
QRect Page::getTargetRect()
{
    return _planner.plan(_pageNumber);
}

bool Page::isCached()
{
    return _cache.isCached(_pageNumber);
}
void Page::setPixmap(const QPixmap &pixmap)
{
    _cache.cache(_pageNumber, pixmap);

    emit pixmapUpdated(this);
}
QPixmap Page::getPixmap()
{
    return _cache.getPixmap(_pageNumber);
}

void Page::setThumbnail(const QPixmap &thumbnail)
{
    _thumbnail = thumbnail;
}
QPixmap Page::getThumbnail()
{
    return _thumbnail;
}

bool Page::isDecodeNeeded()
{
    return _cache.isDecodeNeeded(_pageNumber);
}
void Page::decodeStarted()
{
    _cache.decodeStarted(_pageNumber);
}
void Page::decodeAborted()
{
    _cache.decodeAborted(_pageNumber);
}
void Page::decodeSizeSet(QSize size)
{
    _cache.decodeSizeSet(_pageNumber, size);
}

#include "page.moc"
