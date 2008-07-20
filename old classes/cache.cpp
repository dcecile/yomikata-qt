#include "cache.h"

#include <QMutexLocker>
#include <QThread>
#include <KDebug>

#include "planner.h"

Cache::Cache(Planner &planner, QObject *parent)
    : QObject(parent), _planner(planner), _mutex(QMutex::Recursive)
{
    connect (&_planner, SIGNAL(plannedSizesReset(int, int)), this, SLOT(plannedSizesReset(int, int)));
    _homeThread = QThread::currentThread();
    _numPages = 0;
}

Cache::~Cache()
{
}

void Cache::initialize(int numPages, int pageOne, int pageTwo)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Clear the old entries
    for (int i=0; i<_numPages; i++) {
        delete _entries[i];
    }

    // Reset the number of pages
    _numPages = numPages;

    // Create the new entries
    _entries.resize(_numPages);
    for (int i=0; i<_numPages; i++) {
        _entries[i] = new CacheEntry(i, _manager, _planner);
    }

    // Initialize the manager
    _manager.initialize(numPages, pageOne, pageTwo);
}

void Cache::setCurrentPages(int pageOne, int pageTwo)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let manager handle
    _manager.setCurrentPages(pageOne, pageTwo);
}

int Cache::getPageToPrecache()
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Try to get a page that needs decoding
    int targetPage;
    bool targetWasHot;
    do {
        // Grab the next target
        targetPage = _manager.getPrecacheTarget();

        // See if target is correctly precached
        if (targetPage != -1 && !_entries[targetPage]->isDecodeNeeded()) {

            // This one doesn't need decoding, find a new one
            _manager.setHot(targetPage);
            targetWasHot = true;

        // Else, stick with this target
        } else {
            targetWasHot = false;
        }

    } while (targetWasHot);

    ///@todo fix resizing problem
    if (targetPage != -1 && _entries[targetPage]->isDecoding()) {
        kDebug()<<"skipping precache"<<targetPage;
        return -1;
    }

    // Return the target
    return targetPage;
}

bool Cache::isCached(int pageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let entry handle
    return _entries[pageNumber]->isCached();
}

QPixmap Cache::getPixmap(int pageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let entry handle
    return _entries[pageNumber]->getPixmap();
}

void Cache::cache(int pageNumber, const QPixmap &pixmap)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    kDebug()<<"caching"<<pageNumber;

    // Cache the pixmap
    _entries[pageNumber]->cachePixmap(pixmap);

    // See if any pages need decaching
    list<int> decache;
    _manager.findUnneededPages(&decache);
    for (list<int>::iterator i = decache.begin(); i != decache.end(); i++) {

        // If the pixmap is loaded, decache it
        CacheEntry *entry = _entries[*i];
        kDebug()<<"decaching"<<*i<<entry->isCached();
        if (entry->isCached()) {
            entry->decachePixmap();
        }
    }
}

bool Cache::isDecodeNeeded(int pageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let entry handle
    return _entries[pageNumber]->isDecodeNeeded();
}
void Cache::decodeStarted(int pageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let entry handle
    _entries[pageNumber]->decodeStarted();
}
void Cache::decodeAborted(int pageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    QMutexLocker locker(&_mutex);

    // Let entry handle
    _entries[pageNumber]->decodeAborted();
}
void Cache::decodeSizeSet(int pageNumber, QSize size)
{
    QMutexLocker locker(&_mutex);

    // Let entry handle
    _entries[pageNumber]->decodeSizeSet(size);
}

void Cache::plannedSizesReset(int lowerPageNumber, int upperPageNumber)
{
    Q_ASSERT(QThread::currentThread() == _homeThread);
    ///@todo fix this potential deadlock
    QMutexLocker locker(&_mutex);

    // Let manager handle
    _manager.setCold(lowerPageNumber, upperPageNumber);
}

#include "cache.moc"
