#ifndef CACHEENTRY_H
#define CACHEENTRY_H

#include <QPixmap>

class CacheManager;
class Planner;

class CacheEntry{
public:
    CacheEntry(int pageNumber, CacheManager &manager, Planner &planner);
    ~CacheEntry();

    bool isDecodeNeeded();
    void decodeStarted();
    void decodeSizeSet(const QSize &size);
    void decodeAborted();

    void cachePixmap(const QPixmap &pixmap);
    void decachePixmap();
    QPixmap getPixmap();

    bool isCached();

    bool isDecoding(); ///@todo remove

private:
    void updateMemoryUse(QSize size);
    void updateMemoryUse(QSize size, int depth);
    QSize getPlannedSize();

private:
    static const int DEFAULT_DEPTH = 32;

private:
    int _pageNumber;
    CacheManager &_manager;
    Planner &_planner;

    bool _isDecoding;
    QSize _decodeSize;

    QPixmap _pixmap;
};

#endif
