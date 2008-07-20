#include <QMutexLocker>
#include <QApplication>
#include <KDebug>
#include <QTextStream>

#include <cmath>
#include <algorithm>

#include "pagecache.h"
#include "decodethread.h"

using std::min;
using std::max;

const int PageCache::CACHE_LIMIT = 30 * 1024 * 1024;
const QEvent::Type PageCache::DECODED_EVENT = QEvent::User;
const QEvent::Type PageCache::DEALLOCATE_EVENT = QEvent::Type(int(QEvent::User) + 1);

PageCache::PageCache()
{
    _resizeMode = false;

    _twoPageMode = true;

    _targetPage[0] = -1;
    _targetPage[1] = -1;
    _targetRequested[0] = false;
    _targetRequested[1] = false;
    _primaryPage = 0;
    _numPages = 0;

    _cachedSize = 0;

    _forwardEnabled = false;
    _backwardEnabled = false;
}

void PageCache::setData(int pageNum, const QImage &image, const QSize &fullSize)
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(pageNum >= 0 && pageNum < _numPages);

    // Check that the decode suceeded
    if (image.isNull()) {
        Q_ASSERT(false);
        _pages[pageNum].decodingThread = 0;
        return;
    }

    // Store the image for later translation to pixmap
    _pages[pageNum].image = image;
    _pages[pageNum].fullSize = fullSize;
    _pages[pageNum].decodingThread = 0;

    // Set the wide flag
    _pages[pageNum].isWide = fullSize.width() > fullSize.height();

    // If the page was decoded at the wrong size, decode it over
    // If two pages are being displayed, the screen is split between two non-wide images (probably)
    QSize boundingSize(_displaySize);
    if (_twoPageMode && !_pages[pageNum].isWide) {
        boundingSize.rwidth() /= 2;
    }
    QSize scaledSize(_pages[pageNum].fullSize);
    scaledSize.scale(boundingSize, Qt::KeepAspectRatio);

    _pages[pageNum].isScaled = _pages[pageNum].image.size() == scaledSize;

    kDebug()<<"Decode thread finished page "<<pageNum<<", scaled "<<scaledSize<<"actual "<<_pages[pageNum].image.size()<<"fullsize "<<_pages[pageNum].fullSize<<endl;

    if (_resizeMode) {
        // If we have a wrong-sized resized display page, redecode it
        if ((pageNum == _targetPage[0] || pageNum == _targetPage[1]) &&
                (_pages[pageNum].image.size() != _pages[pageNum].fullSize)) {
            requestPage(pageNum);
        }
    } else if (!_pages[pageNum].isScaled) {
        // If we have a wrong-sized page (in non-resize mode), redecode it
        requestPage(pageNum);
    }

    // Notify the main thread that this page is finished and may or may not need displaying
    if (_decodedPages.empty()) {
        QApplication::postEvent(this, new QEvent(DECODED_EVENT));
    }
    _decodedPages.insert(pageNum);
}

void PageCache::pageDecoded(int pageNum)
{
    QTime time;
    time.start();

    bool previouslyCached = !_pages[pageNum].pixmap.isNull();

    // Take out the memory use of the previous pixmap
    if (previouslyCached) {
        _cachedSize -= memoryUse(pageNum);
    }

    // Translate the image
    _pages[pageNum].pixmap = QPixmap::fromImage(_pages[pageNum].image);

    // Deallocate the image
    _pages[pageNum].image = QImage();

    //kDebug()<<"QImage to QPixmap conversion: "<<time.elapsed()<<" ms"<<endl;

    // Add the memory use of this page's pixmap and mark it as cached
    _cachedSize += memoryUse(pageNum);
    if (!previouslyCached) {
        _pagesCached.insert(pageNum);
    }

    kDebug()<<"Page "<<pageNum<<" decoded: "<<_pages[pageNum].loadingTime.elapsed()<<" ms - "<<_pages[pageNum].pixmap.size()<<endl;
    kDebug()<<"Cache size: "<<(_cachedSize/(1024))<<" KB"<<endl;

    // Check that the image to pixmap conversion suceeded
    if (_pages[pageNum].pixmap.isNull()) {
        Q_ASSERT(false);
    }

    // Check if the this page needs displaying
    if (!_twoPageMode || _targetPage[1] == -1) {
        if (pageNum == _targetPage[0]) {
            emit showOnePage(_pages[pageNum].pixmap, _targetPage[0], _numPages);
        }
    } else {
        // If two pages are being displayed, display them both at once
        // If one of the pages is wide, only show the primary page
        for (int i=0; i<2; i++) {
            if (pageNum == _targetPage[i]) {
                if (_pages[pageNum].isWide) {
                    //kDebug()<<"Page "<<pageNum<<", target 0 wide"<<endl;
                    _targetPage[0] = _targetPage[_primaryPage];
                    _targetPage[1] = -1;

                    if (pageNum == _targetPage[0] || !_pages[_targetPage[0]].pixmap.isNull()) {
                        emit showOnePage(_pages[_targetPage[0]].pixmap, _targetPage[0], _numPages);
                    }
                    updateEnabledActions();
                } else if (!_pages[_targetPage[!i]].pixmap.isNull()) {
                    emit showTwoPages(_pages[_targetPage[0]].pixmap, _pages[_targetPage[1]].pixmap, _targetPage[0], _targetPage[1], _numPages);
                } else {
                    kDebug()<<"Can't display "<<pageNum<<endl;
                }
                break;
            }
        }
    }
}

bool PageCache::event(QEvent *event)
{
    QMutexLocker locker(&_mutex);

    switch (event->type()) {
    case DECODED_EVENT:
        // Transcoded and maybe display the queued page(s)
        for (set<int>::iterator i = _decodedPages.begin(); i != _decodedPages.end(); i++) {
            pageDecoded(*i);
        }
        _decodedPages.clear();
        return true;

    case DEALLOCATE_EVENT:
        // Deallocate the queued pages
        for (set<int>::iterator i = _pagesToDeallocate.begin(); i != _pagesToDeallocate.end(); i++) {
            deallocatePage(*i);
        }
        _pagesToDeallocate.clear();
        return true;

    default:
        return false;
    }
}

int PageCache::memoryUse(int pageNum)
{
    const QPixmap &pixmap(_pages[pageNum].pixmap);
    if (pixmap.isNull()) {
        return 0;
    } else {
        return (pixmap.width() * pixmap.height() * pixmap.depth()) / 8;
    }
}

void PageCache::scheduleNextDecode(DecodeThread *decodingThread, int *nextPageNum, QString *path, QSize *fullSize, QSize *boundingSize)
{
    QMutexLocker locker(&_mutex);

    kDebug()<<"Scheduling next decode"<<endl;

    int next = -1;

    do {
        next = -1;
        printCached();

        // See if the primary page needs decoding
        if (_targetRequested[_primaryPage]) {
            next = _targetPage[_primaryPage];
            _targetRequested[_primaryPage] = false;

        // See if the non-primary page needs decoding
        } else if (_targetRequested[!_primaryPage]) {
            next = _targetPage[!_primaryPage];
            _targetRequested[!_primaryPage] = false;

        // Pick the next best page that's close to the target pages
        } else {
            kDebug()<<"Finding next best page"<<endl;
            next = findNextBestPage();

            // If nothing good was found, wait until something changes
            if (next == -1) {
                kDebug()<<"Waiting..."<<endl;
                _noRequestedPages.wait(&_mutex);
                kDebug()<<"Waking..."<<endl;

                // If the cache has been reset, just return
                if (_numPages == 0) {
                    return;
                }
            }
        }
    } while (next == -1);

    kDebug()<<"Found next page, "<<next<<endl;

    // Tell the thread what to start decoding
    *nextPageNum = next;
    *path = _pages[next].path;
    *fullSize = _pages[next].fullSize;

    // Pick a bounding size

    // In resize mode, decode the current page(s) at full resolution
    if (_resizeMode && (next == _targetPage[0] || next == _targetPage[1])) {
        *boundingSize = QSize();

    // If the page is wide or this is single page mode, fit to the full display size
    } else if (!_twoPageMode || _pages[next].isWide) {
        *boundingSize = _displaySize;

    // Otherwise, the screen is split between to images (probably)
    } else {
        *boundingSize = _displaySize;
        boundingSize->rwidth() /= 2;
    }

    // Remember the bounding size
    _pages[next].decodingBoundingSize = *boundingSize;

    // As of now, the page is scaled
    _pages[next].isScaled = true;

    // Remember this thread
    _pages[next].decodingThread = decodingThread;

    // This page is no longer queued
    _pages[next].isRequested = false;

    // Keep the request iterator valid
    if (_requestedPosition != _pagesRequested.end() && *_requestedPosition == next) {
        _requestedPosition++;
    }
    _pagesRequested.erase(next);
}

int PageCache::findNextBestPage()
{
    // If everything's decoded, there is no choice
    if (_pagesRequested.empty()) {
        return -1;
    }

    // Keep track of the page picked and how "good" it was
    int picked;
    int pickedDist;

    // If there's nothing after, pick the first page before
    if (_requestedPosition == _pagesRequested.end()) {
        picked = *_pagesRequested.rbegin();
        pickedDist = abs(picked + 1 - _targetPage[0]) / 2;

    } else {
        // If there's nothing before, pick the first page after
        set<int>::iterator temp(_requestedPosition);
        temp--;
        if (temp == _pagesRequested.end()) {
            picked = *_requestedPosition;
            pickedDist = abs(picked - 1 - max(_targetPage[0], _targetPage[1])) / 2;

        } else {
            // See how far the pages are before the first page
            int distBefore = abs(*temp + 1 - _targetPage[0]) / 2;

            // See how far the pages are after the second page
            int distAfter = abs(*_requestedPosition - 1 - max(_targetPage[0], _targetPage[1])) / 2;

            // Pick the best page to decode first
            // If there's a tie and we're paging backward, pick the page before
            if (distBefore < distAfter || (distBefore == distAfter && _primaryPage == 1)) {
                picked = *temp;
                pickedDist = distBefore;
            } else {
                picked = *_requestedPosition;
                pickedDist = distAfter;
            }
        }
    }

    // See if the cache is full with this page deallocated
    // See if the next picked is better to have decoded than something cached
    // Don't decrease the cache size below 6
    int tempCachedSize = _cachedSize - memoryUse(picked);
    if (tempCachedSize > CACHE_LIMIT) {

        bool abort = false;
        int tempNext;
        QList<int> deallocate;

        set<int>::iterator front = _pagesCached.begin();
        set<int>::reverse_iterator back = _pagesCached.rbegin();

        kDebug()<<"Cache full, accessing page "<<picked<<endl;

        do {
            // See how far the front page is before the first page
            int distFront = abs(*front + 1 - _targetPage[0]) / 2;

            // See how far the back page is after the second page
            int distBack = abs(*back - 1 - max(_targetPage[0], _targetPage[1])) / 2;

            // Pick the best page to deallocate first
            // If there's a tie and we're paging backward, deallocate the page after
            int furtherDist;
            if (distFront < distBack || (distFront == distBack && _primaryPage == 1)) {
                tempNext = *back++;
                furtherDist = distBack;
            } else {
                tempNext = *front++;
                furtherDist = distFront;
            }

            // Don't deallocate if the picked page isn't good enough
            if (tempNext != pickedDist && furtherDist <= pickedDist) {
                abort = true;
                kDebug()<<"Aborting, found cached page "<<tempNext<<endl;

            // Else, add the page to the list, decrease the cache size
            } else {
                deallocate<<tempNext;
                tempCachedSize -= memoryUse(tempNext);
            }

        } while (!abort && tempCachedSize > CACHE_LIMIT);

        if (!abort) {
            // If this page is already cached, deallocate it
            // Decache the other page
            for (QList<int>::iterator i = deallocate.begin(); i != deallocate.end(); i++) {
                _pagesToDeallocate.insert(*i);
            }
            _pagesToDeallocate.insert(picked);

            // Schedule the deallocation
            QApplication::postEvent(this, new QEvent(DEALLOCATE_EVENT));

            // Have the already picked page be decoded

        } else {
            // Wait for some stuff to change
            picked = -1;
        }
    }

    return picked;
}

void PageCache::deallocatePage(int pageNum)
{
    bool cached = !_pages[pageNum].pixmap.isNull();

    if (cached) {
        Q_ASSERT(_pagesCached.count(pageNum) == 1);
        //Q_ASSERT(_pagesRequested.count(pageNum) == 0);
        kDebug()<<"Deallocating page "<<pageNum<<endl;

        // Take out the memory use of the pixmap
        _cachedSize -= memoryUse(pageNum);
        _pages[pageNum].pixmap = QPixmap();
        _pagesCached.erase(pageNum);

        // The page will need decoding again
        _pagesRequested.insert(pageNum);
    }
}

void PageCache::requestPage(int pageNum)
{
    Q_ASSERT(pageNum >= 0 && pageNum < _numPages);

    kDebug()<<"Requesting page "<<pageNum<<endl;
    if (_pages[pageNum].decodingThread != 0) {
        _pages[pageNum].decodingThread->abortDecode();
    }
    _pagesRequested.insert(pageNum);
    _pages[pageNum].isRequested = true;
    _pages[pageNum].loadingTime.restart();

    /*
    // Only request the page if it isn't already requested and it isn't scaled correctly
    if (_resizeMode) {
        if (pageNum == _targetPage[0] || pageNum == _targetPage[1]) {
            if (!_pages[pageNum].pixmap.isNull() && _pages[pageNum].pixmap.size() == _pages[pageNum].fullSize) {
                // The full resolution is already decoded
                if (_pages[pageNum].isRequested) {
                    if (*_requestedPosition == pageNum) {
                        _requestedPosition++;
                    }
                    _pagesRequested.erase(pageNum);
                }
                if (_pages[pageNum].decodingThread) {
                    _pages[pageNum].decodingThread->abortDecode();
                }
            } else if (!_pages[pageNum].isRequested) {
                kDebug()<<"Requesting page "<<pageNum<<endl;
                if (_pages[pageNum].decodingThread != 0) {
                    _pages[pageNum].decodingThread->abortDecode();
                }
                _pagesRequested.insert(pageNum);
                _pages[pageNum].isRequested = true;
                _pages[pageNum].loadingTime.restart();
            }
        }
    } else {
        if (!_pages[pageNum].isRequested && !_pages[pageNum].isScaled) {
            kDebug()<<"Requesting page "<<pageNum<<endl;
            if (_pages[pageNum].decodingThread != 0) {
                _pages[pageNum].decodingThread->abortDecode();
            }
            _pagesRequested.insert(pageNum);
            _pages[pageNum].isRequested = true;
            _pages[pageNum].loadingTime.restart();
        }
    }*/
}

void PageCache::reset()
{
    QMutexLocker locker(&_mutex);

    // Clear the pages list
    _pages.clear();

    // Clear requested pages
    _pagesRequested.clear();

    // Clear cached pages
    _pagesCached.clear();

    // Reinit
    _targetPage[0] = -1;
    _targetPage[1] = -1;
    _targetRequested[0] = false;
    _targetRequested[1] = false;
    _primaryPage = 0;
    _numPages = 0;

    _cachedSize = 0;

    _forwardEnabled = false;
    _backwardEnabled = false;
    emit forwardEnabled(false);
    emit backwardEnabled(false);

    // Wake any waiting threads
    _noRequestedPages.wakeAll();
}

void PageCache::initialize(int startPageNum, const QStringList &files)
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(files.size() > 0);

    _numPages = files.size();

    kDebug()<<"Total pages: "<<_numPages<<endl;

    // Create a page entry for each file
    Page newPage;
    for (QStringList::const_iterator i = files.begin(); i != files.end(); i++) {
        newPage.path = *i;
        _pages.append(newPage);

        if (i == files.begin()) {kDebug()<<"Path: "<<_pages.back().path<<endl;}
    }

    // Choose the starting page(s)
    _targetPage[0] = startPageNum;
    if (_twoPageMode) {
        if (_targetPage[0] + 1 < _numPages) {
            _primaryPage = 0;
            _targetPage[1] = startPageNum + 1;
        } else {
            _targetPage[1] = -1;
        }
    }

    // Synch the actions
    updateEnabledActions();

    // Request all pages
    _targetRequested[0]  = true;
    _targetRequested[1]  = _targetPage[1] != -1;
    requestPage(_targetPage[0]);
    for (int i = 0; i < _numPages; i++) {
        requestPage(i);
    }

    _requestedPosition = _pagesRequested.lower_bound(_targetPage[0]);

    // The request position is changed, wake any waiting threads
    _noRequestedPages.wakeAll();
}

void PageCache::changePage()
{
    // Change the page to the current target page(s)

    // A page change means that resize mode is inactive
    if (_resizeMode) {
        _resizeMode = false;

        _pagesRequested.clear();

        // Reasses whether each page is scaled correctly
        // Request all unscaled pages
        QSize boundingSize, scaledSize;
        for (int i = 0; i < _numPages; i++) {
            if (_pages[i].fullSize.isValid()) {
                boundingSize = _displaySize;
                if (_twoPageMode && !_pages[i].isWide) {
                    boundingSize.rwidth() /= 2;
                }
                scaledSize = _pages[i].fullSize;
                scaledSize.scale(boundingSize, Qt::KeepAspectRatio);

                _pages[i].isScaled = _pages[i].image.size() == scaledSize;
                kDebug()<<"Page "<<i<<" is scaled: "<< _pages[i].isScaled<<endl;
            }

            if (!_pages[i].isScaled) {
                requestPage(i);
            } else {
                _pages[i].isRequested = false;
            }
        }

    }

    if (_targetPage[1] != -1) {
        // If one of the pages is wide, only show the primary page
        if (_pages[_targetPage[0]].isWide || _pages[_targetPage[1]].isWide) {
            kDebug()<<"Page "<<_targetPage[0]<<" or page "<<_targetPage[1]<<" is wide"<<endl;
            _targetPage[0] = _targetPage[_primaryPage];
            _targetPage[1] = -1;
        }
    }

    // Make sure the actions are in synch
    updateEnabledActions();

    bool pageNumberShown = false;

    // If the pages are loaded, show them
    if (!_twoPageMode || _targetPage[1] == -1) {
        if (!_pages[_targetPage[0]].pixmap.isNull()) {
            emit showOnePage(_pages[_targetPage[0]].pixmap, _targetPage[0], _numPages);
            pageNumberShown = true;
        }
    } else {
        if (!_pages[_targetPage[0]].pixmap.isNull() && !_pages[_targetPage[1]].pixmap.isNull()) {
            emit showTwoPages(_pages[_targetPage[0]].pixmap, _pages[_targetPage[1]].pixmap, _targetPage[0], _targetPage[1], _numPages);
            pageNumberShown = true;
        }
    }

    if (!pageNumberShown) {
        // Show the page number
        emit showPageNumber(_targetPage[0], _targetPage[1], _numPages);
    }

    kDebug()<<"Changing page "<<_targetPage[0]<<" ("<<_targetRequested[0]<<") "<<_targetPage[1]<<" ("<<_targetRequested[1]<<")"<<endl;

    // Reset the request position
    _requestedPosition = _pagesRequested.lower_bound(_targetPage[0]);

    // The request position is changed, wake any waiting threads
    _noRequestedPages.wakeAll();
}

void PageCache::setDisplaySize(const QSize &displaySize)
{
    QMutexLocker locker(&_mutex);

    // Store the display size
    _displaySize = displaySize;

    if (_numPages != 0 && !_resizeMode) {
        _resizeMode = true;

        // Stop all requested decodes
        _pagesRequested.clear();

        // Redecode the current page(s) at full resolution
        requestPage(_targetPage[0]);
        if (_targetPage[1] != -1) {
            requestPage(_targetPage[1]);
        }
        _requestedPosition = _pagesRequested.lower_bound(_targetPage[0]);

        // The request position is changed, wake any waiting threads
        _noRequestedPages.wakeAll();
    }
}

void PageCache::setTwoPageMode(bool)
{
    Q_ASSERT(false);
}

void PageCache::printCached()
{
    QString temp;
    QTextStream stream(&temp);
    for (set<int>::iterator i=_pagesCached.begin(); i!=_pagesCached.end(); i++) {
        stream<<*i<<' ';
    }
    kDebug()<<"Cached "<<_pagesCached.size()<<": "<<temp<<endl;
}

void PageCache::navigateForward()
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_targetPage[1] != -1) {
            _targetPage[0] += 2;
        } else {
            _targetPage[0]++;
        }

        if (_targetPage[0] + 1 < _numPages) {
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Change the page
    changePage();
}

void PageCache::navigateBackward()
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_backwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_targetPage[0] - 2 < 0) {
            _targetPage[0]--;
            _targetPage[1] = -1;

        } else {
            _targetPage[0] -= 2;
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 1;
        }

    } else {
        _targetPage[0]--;
    }

    // Change the page
    changePage();
}

void PageCache::navigateForwardOnePage()
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        _targetPage[0]++;

        if (_targetPage[0] + 1 < _numPages) {
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Change the page
    changePage();
}

void PageCache::navigateToStart()
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_backwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        _targetPage[0] = 0;

        if (_numPages > 1) {
            _targetPage[1] = 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0] = 0;
    }

    // Change the page
    changePage();
}

void PageCache::navigateToEnd()
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_numPages > 1) {
            _targetPage[1] = _numPages - 1;
            _targetPage[0] = _numPages - 2;
            _primaryPage = 1;

        } else {
            _targetPage[0] = 0;
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Change the page
    changePage();
}

void PageCache::updateEnabledActions()
{
    bool oldForward = _forwardEnabled;
    bool oldBackward = _backwardEnabled;

    // See if there's room to navigate forwards or backwards
    if (_numPages == 0) {
        _forwardEnabled = false;
        _backwardEnabled = false;
    } else {
        _backwardEnabled = _targetPage[0] > 0;
        if (_targetPage[1] == -1) {
            _forwardEnabled = _targetPage[0] + 1 < _numPages;
        } else {
            _forwardEnabled = _targetPage[1] + 1 < _numPages;
        }
    }

    // Emit the signals only of the state changed
    if (_forwardEnabled != oldForward) {
        emit forwardEnabled(_forwardEnabled);
    }
    if (_backwardEnabled != oldBackward) {
        emit backwardEnabled(_backwardEnabled);
    }
}

#include "pagecache.moc"
