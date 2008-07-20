#include "cacheentry.h"

#include <KDebug>

#include "cachemanager.h"
#include "planner.h"

CacheEntry::CacheEntry(int pageNumber, CacheManager &manager, Planner &planner)
    :_pageNumber(pageNumber), _manager(manager), _planner(planner), _isDecoding(false)
{
}
CacheEntry::~CacheEntry()
{
}

bool CacheEntry::isDecodeNeeded()
{
    // If not decoding and no cached pixmap, decode needed
    if (_pixmap.isNull() && !_isDecoding) {
        return true;
    }

    // If decoding and no size set, don't decode
    if (_isDecoding && !_decodeSize.isValid()) {
        return false;
    }

    QSize targetSize = getPlannedSize();

    // If the pixmap has been decoded and is the correct size
    if (!_pixmap.isNull() && _pixmap.size() == targetSize) {
        return false;
    }

    // If during decoding and size is set,
    //  decode needed if size is wrong
    if (_isDecoding) {
        return _decodeSize != targetSize;

    // Otherwise, the pixmap is the wrong size
    } else {
        return true;
    }
}

void CacheEntry::decodeStarted()
{
    // Assert something good isn't already cached
    Q_ASSERT(isDecodeNeeded());

    ///@todo allow multiple concurrent decodes
    Q_ASSERT(!_isDecoding);

    // Decode started
    _isDecoding = true;
    _decodeSize = QSize();

    // Update the memory use
    updateMemoryUse(getPlannedSize());
}
void CacheEntry::decodeSizeSet(const QSize &size)
{
    // Assert decode has been started
    Q_ASSERT(_isDecoding);

    // Decode size set
    _decodeSize = size;

    // Update the memory use
    QSize plannedSize = getPlannedSize();
    updateMemoryUse(plannedSize);

    // Check that the size is right
    if (_decodeSize != plannedSize) {
        _manager.setCold(_pageNumber);
    }
}
void CacheEntry::decodeAborted()
{
    Q_ASSERT(_isDecoding);

    // Decode stopped
    _isDecoding = false;

    // Assume this page is now cold
    _manager.setCold(_pageNumber);

    // Update with the accurate memory use
    if (_pixmap.isNull()) {
        _manager.setMemoryUse(_pageNumber, 0);
    } else {
        updateMemoryUse(_pixmap.size(), _pixmap.depth());
    }
}

void CacheEntry::cachePixmap(const QPixmap &pixmap)
{
    Q_ASSERT(_isDecoding);

    // No longer decoding
    _isDecoding = false;

    // Pixmap cached
    _pixmap = pixmap;

    // Update the memory use
    updateMemoryUse(_pixmap.size(), _pixmap.depth());

    // Check that the size is right
    if (_pixmap.size() != getPlannedSize()) {
        _manager.setCold(_pageNumber);
    }
}
void CacheEntry::decachePixmap()
{
    Q_ASSERT(!_pixmap.isNull());

    // Pixmap decached
    _pixmap = QPixmap();

    // Assume this page is now cold
    _manager.setCold(_pageNumber);

    // Update with the accurate memory use
    if (!_isDecoding) {
        _manager.setMemoryUse(_pageNumber, 0);
    }
}
QPixmap CacheEntry::getPixmap()
{
    // Give the pixmap
    Q_ASSERT(!_pixmap.isNull());
    return _pixmap;
}

bool CacheEntry::isCached()
{
    // Cached if the pixmap is valid
    return !_pixmap.isNull();
}

bool CacheEntry::isDecoding()
{
    return _isDecoding;
}

void CacheEntry::updateMemoryUse(QSize size)
{
    // Guess the depth
    updateMemoryUse(size, DEFAULT_DEPTH);
}
void CacheEntry::updateMemoryUse(QSize size, int depth)
{
    // Measure size in bytes
    int newMemoryUse = (size.width() * size.height() * depth) / 8;

    // Notify the manager
    _manager.setMemoryUse(_pageNumber, newMemoryUse);
}

QSize CacheEntry::getPlannedSize()
{
    return _planner.plan(_pageNumber).size();
}
