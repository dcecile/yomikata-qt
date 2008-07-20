#include "cachemanager.h"

#include <QDebug>
#include <KDebug>

#include <cmath>

const int CacheManager::CACHE_LIMIT = 10 * 1024 * 1024;

CacheManager::CacheManager()
{
    _numPages = 0;
    _rangeSet = false;
    _allStorage = 0;
    _coldStorage = 0;
}

CacheManager::~CacheManager()
{
}

void CacheManager::initialize(int numPages, int pageOne, int pageTwo)
{
    // Set number of pages
    _numPages = numPages;

    // Nothing in memory
    _allStorage = 0;
    _coldStorage = 0;
    _storageMap.clear();

    // No pages hot
    _rangeSet = false;

    // Set the current pages
    if (pageTwo == -1) {
        _lowerCurrent = _upperCurrent = pageOne;
    } else {
        _lowerCurrent = pageOne;
        _upperCurrent = pageTwo;
    }

    check();
}

void CacheManager::setCurrentPages(int pageOne, int pageTwo)
{
    Q_ASSERT(pageOne >= 0 && pageOne < _numPages);

    // Set the current pages
    if (pageTwo == -1) {
        _lowerCurrent = _upperCurrent = pageOne;
    } else {
        Q_ASSERT(pageTwo >= 0 && pageTwo < _numPages);
        _lowerCurrent = pageOne;
        _upperCurrent = pageTwo;
    }

    // Make sure the range includes the current pages
    if (_rangeSet && (_lowerCurrent > _upperHot || _upperCurrent < _lowerHot )) {
        _rangeSet = false;
        _coldStorage = _allStorage;
    }

    // Make sure the range is balanced
    if (_rangeSet) {
        balance();
    }

    check();
}

void CacheManager::setMemoryUse(int pageNumber, int memoryUse)
{
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);

    // Check for an entry
    StorageMap::iterator found = _storageMap.find(pageNumber);

    // If zero, remove the entry
    if (memoryUse == 0) {
        Q_ASSERT(found != _storageMap.end());

        // Assert this is a cold page
        Q_ASSERT(!_rangeSet || pageNumber < _lowerHot || pageNumber > _upperHot);

        // Update the total storage
        _allStorage -= found->second;
        // And cold storage
        _coldStorage -= found->second;

        // Remove the entry
        _storageMap.erase(found);

    // Otherwise, update the storage
    } else {

        // Save the previous value and update the page's storage
        int prevStorage;
        if (found != _storageMap.end()) {
            prevStorage = found->second;
            found->second = memoryUse;
        } else {
            prevStorage = 0;
            _storageMap[pageNumber] = memoryUse;
        }

        // Update the total storage
        _allStorage += memoryUse - prevStorage;

        // If in cold storage, update that too
        if (!_rangeSet || pageNumber < _lowerHot || pageNumber > _upperHot) {
            _coldStorage += memoryUse - prevStorage;
        }
    }

    check();
}

int CacheManager::getPrecacheTarget() const
{
    // If the range isn't defined, just give the lower current page
    if (!_rangeSet) {
        return _lowerCurrent;
    }

    // If lower current isn't in the range, it's next
    if (_lowerCurrent < _lowerHot) {
        return _lowerCurrent;
    }

    // Or if upper current isn't in the range, it's next
    if (_upperCurrent > _upperHot) {
        return _upperCurrent;
    }

    // Don't precache if the cache is full of good pixmaps
    if (_allStorage - _coldStorage > CACHE_LIMIT) {
        return -1;
    }

    // If whole book buffered, nothing to do
    if (_lowerHot == 0 && _upperHot == _numPages -1) {
        return -1;
    }

    // If can't go lower, go up
    if (_lowerHot == 0) {
        return _upperHot + 1;
    }

    // If can't go higher, go down
    if (_upperHot == _numPages -1) {
        return _lowerHot - 1;
    }

    // Otherwise, pick the the direction that's short on pages
    int bufferUp = _upperHot - _upperCurrent;
    int bufferDown = _lowerCurrent - _lowerHot;
    if (bufferUp <= bufferDown) {
        return _upperHot + 1;
    } else {
        return _lowerHot - 1;
    }
}

void CacheManager::setCold(int pageNumber)
{
    setCold(pageNumber, pageNumber);
}

void CacheManager::setCold(int lowerPageNumber, int upperPageNumber)
{
    Q_ASSERT(lowerPageNumber >= 0 && lowerPageNumber < _numPages);
    Q_ASSERT(upperPageNumber >= 0 && upperPageNumber < _numPages);
    Q_ASSERT(lowerPageNumber <= upperPageNumber);

    // If the range isn't set, there's nothing to do
    if (!_rangeSet) {
        return;
    }

    // If the current pages are cold, reset the range
    if (lowerPageNumber <= _lowerCurrent && upperPageNumber >= _upperCurrent) {
        _rangeSet = false;
        _coldStorage = _allStorage;

    // If the cold pages overlap the lower hot pages
    } else if (lowerPageNumber >= _upperCurrent && lowerPageNumber <= _upperHot) {
        // Truncate the hot range
        unbound(lowerPageNumber, _upperHot);
        // Make sure its still balanced
        balance();

    // If the cold pages overlap the upper hot pages
    } else if (upperPageNumber <= _lowerCurrent && upperPageNumber >= _lowerHot) {
        // Truncate the hot range
        unbound(_lowerHot, upperPageNumber);
        // Make sure its still balanced
        balance();
    }

    check();
}

void CacheManager::setHot(int pageNumber)
{
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);

    // Expand the hot range
    if (!_rangeSet) {
        Q_ASSERT(pageNumber == _lowerCurrent);
        _lowerHot = _upperHot = _lowerCurrent;
        _rangeSet = true;
    } else if (pageNumber == _lowerHot - 1) {
        _lowerHot--;
    } else {
        Q_ASSERT(pageNumber == _upperHot + 1);
        _upperHot++;
    }

    // Find the entry
    StorageMap::iterator found = _storageMap.find(pageNumber);
    Q_ASSERT(found != _storageMap.end());

    // Remove from cold storage
    _coldStorage -= found->second;

    check();
}

void CacheManager::findUnneededPages(list<int> *results)
{
    // Start with a blank list
    results->clear();

    // If the cache is below the limit, nothing to be done
    if (_allStorage < CACHE_LIMIT) {
        return;
    }

    // Keep a prediction of the storage
    int predictedStorage = _allStorage;

    // Start from the exterior, working in
    StorageMap::iterator lower = _storageMap.begin();
    StorageMap::reverse_iterator upper = _storageMap.rbegin();

    // Loop until the predicted storage is above the cache limit by one page
    bool lastPageFound = false;
    while (!lastPageFound) {

        //kDebug()<<lower->first<<upper->first;
        Q_ASSERT(lower != _storageMap.end());
        Q_ASSERT(upper != _storageMap.rend());

        // The page to decache is the one farthest from the current pages
        int pageNumber;
        int pageStorage;
        int distanceUp = upper->first - _upperCurrent;
        int distanceDown = _lowerCurrent - lower->first;

        // The lower page is further
        if (distanceDown >= distanceUp) {
            pageNumber = lower->first;
            pageStorage = lower->second;
            lower++;

        // The upper page is further
        } else {
            pageNumber = upper->first;
            pageStorage = upper->second;
            upper++;
        }

        Q_ASSERT(lower != _storageMap.end());
        Q_ASSERT(upper != _storageMap.rend());

        // Test what would happen if the page was decached
        predictedStorage -= pageStorage;

        // If this was the last page, don't actually decache it
        if (predictedStorage < CACHE_LIMIT) {
            lastPageFound = true;

        // Otherwise do actually decache it
        } else {
            results->push_back(pageNumber);
        }
    }

    check();
}

void CacheManager::balance()
{
    // Calculate buffer above and bellow the current pages
    int bufferUp = _upperHot - _upperCurrent;
    int bufferDown = _lowerCurrent - _lowerHot;

    // If the buffer above is bigger by more than one
    //  and the buffer below isn't constrained, truncate the buffer above
    if (bufferUp > bufferDown + 1 && _lowerHot != 0) {
        unbound(_upperHot - (bufferUp - bufferDown - 2), _upperHot);

    // If the buffer below is bigger by more than one
    //  and the buffer above isn't constrained, truncate the buffer below
    } else if (bufferDown > bufferUp + 1 && _upperHot != _numPages -1) {
        unbound(_lowerHot, _lowerHot + (bufferDown - bufferUp - 2));
    }
}

void CacheManager::unbound(int lower, int upper)
{
    Q_ASSERT(lower <= upper);

    // Truncate from the bottom
    if (lower == _lowerHot) {
        _lowerHot = upper + 1;

    // Or truncate from the top
    } else {
        Q_ASSERT(upper == _upperHot);
        _upperHot = lower - 1;
    }

    // Assert the current pages are still in the range
    Q_ASSERT((_lowerCurrent >= _lowerHot && _lowerCurrent <= _upperHot)
            || (_upperCurrent >= _lowerHot && _upperCurrent <= _upperHot));

    // Find the start and end of the truncation
    StorageMap::iterator i = _storageMap.find(lower);
    StorageMap::iterator j = _storageMap.find(upper);
    Q_ASSERT(i != _storageMap.end());
    Q_ASSERT(j != _storageMap.end());

    // Put the truncated pages into cold storage
    int size = 1;
    for(; i != j; i++) {
        _coldStorage += i->second;
        size++;
    }
    _coldStorage += i->second;

    // Assert all the pages were in the range
    Q_ASSERT(size == upper - lower + 1);
}

void CacheManager::check()
{
    if (_numPages == 0) {
        return;
    }

    // Check the current pages
    //kDebug()<<"current"<<_lowerCurrent<<_upperCurrent;
    Q_ASSERT(_lowerCurrent >= 0 && _lowerCurrent < _numPages);
    Q_ASSERT(_upperCurrent >= 0 && _upperCurrent < _numPages);
    Q_ASSERT(_lowerCurrent <= _upperCurrent);

    // Check the hot range
    //kDebug()<<"range"<<_rangeSet<<_lowerHot<<_upperHot;
    if (_rangeSet) {
        Q_ASSERT(_lowerHot >= 0 && _lowerHot < _numPages);
        Q_ASSERT(_upperHot >= 0 && _upperHot < _numPages);
        Q_ASSERT(_lowerHot <= _lowerHot);
        Q_ASSERT((_lowerCurrent >= _lowerHot && _lowerCurrent <= _upperHot)
                || (_upperCurrent >= _lowerHot && _upperCurrent <= _upperHot));

        int bufferUp = _upperHot - _upperCurrent;
        int bufferDown = _lowerCurrent - _lowerHot;
        Q_ASSERT((_lowerHot == 0 && bufferDown < bufferUp - 1)
                || (_upperHot == _numPages -1 && bufferUp < bufferDown - 1)
                || abs(bufferUp - bufferDown) <= 1);
    }

    // Check the storage calculations
    int testAllStorage = 0;
    int testColdStorage = 0;
    for(StorageMap::iterator i = _storageMap.begin(); i != _storageMap.end(); i++) {
        testAllStorage += i->second;
        if (!_rangeSet || i->first < _lowerHot || i->first > _upperHot) {
            testColdStorage += i->second;
        }
    }
    //kDebug()<<"all"<<testAllStorage<<_allStorage;
    //kDebug()<<"cold"<<testColdStorage<<_coldStorage;
    Q_ASSERT(testAllStorage == _allStorage);
    Q_ASSERT(testColdStorage == _coldStorage);

    //kDebug()<<"check passed";
}
