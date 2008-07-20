#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <list>
#include <map>

using std::list;
using std::map;

class CacheManager{
public:
    CacheManager();
    ~CacheManager();

    void initialize(int numPages, int pageOne, int pageTwo);

    void setCurrentPages(int pageOne, int pageTwo);

    void setMemoryUse(int pageNumber, int memoryUse);

    int getPrecacheTarget() const;

    void setCold(int pageNumber);
    void setCold(int lowerPageNumber, int upperPageNumber);
    void setHot(int pageNumber);

    void findUnneededPages(list<int> *results);

private:
    void balance();
    void unbound(int lower, int upper);

    void check();

private:
    static const int CACHE_LIMIT;

private:
    typedef map<int,int> StorageMap;
    StorageMap _storageMap;

    int _numPages;

    int _upperCurrent;
    int _lowerCurrent;

    bool _rangeSet;
    int _upperHot;
    int _lowerHot;

    int _allStorage;
    int _coldStorage;
};

#endif
