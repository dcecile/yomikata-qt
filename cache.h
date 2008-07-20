#ifndef CACHE_H
#define CACHE_H

#include <QObject>
#include <QPixmap>
#include <QMutex>

#include <map>
#include <vector>

#include "cachemanager.h"
#include "cacheentry.h"

using std::map;
using std::vector;

class Planner;
class QThread;

/**
 * @defgroup mod_cache Cache module
 *
 */

/**
 * @ingroup mod_cache
 * @brief Caches images
 */
class Cache : public QObject
{
    Q_OBJECT

public:
    Cache(Planner &planner, QObject *parent);
    ~Cache();

    void initialize(int numPages, int pageOne, int pageTwo);

    void setCurrentPages(int pageOne, int pageTwo);

    int getPageToPrecache();

    bool isCached(int pageNumber);
    void cache(int pageNumber, const QPixmap &pixmap);
    QPixmap getPixmap(int pageNumber);

    bool isDecodeNeeded(int pageNumber);
    void decodeStarted(int pageNumber);
    void decodeAborted(int pageNumber);
    void decodeSizeSet(int pageNumber, QSize size);

private slots:
    void plannedSizesReset(int lowerPageNumber, int upperPageNumber);

private:
    Planner &_planner;

    QThread *_homeThread;
    QMutex _mutex;

    CacheManager _manager;
    vector<CacheEntry *> _entries;

    int _numPages;
};

#endif
