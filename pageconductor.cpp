#include "pageconductor.h"

#include <QMutexLocker>
#include <KDebug>

PageConductor::PageConductor(QObject *parent)
    : QObject(parent)
{
    _twoPageMode = true;
}


PageConductor::~PageConductor()
{
}

void PageConductor::initialize(int numPages)
{
    QMutexLocker locker(&_mutex);

    _pages.clear();

    _numPages = numPages;
    _primaryPage = 0;
    _targetPage[0] = 0;
    if (_twoPageMode && _numPages > 1) {
        _targetPage[1] = 1;
    } else {
        _targetPage[1] = -1;
    }

    _pages.resize(_numPages);
    PageInfo defaultPage;
    defaultPage.prevDouble = -1;
    defaultPage.nextDouble = -1;
    fill(_pages.begin(), _pages.end(), defaultPage);
}

int PageConductor::getPageOne()
{
    QMutexLocker locker(&_mutex);

    if (_targetPage[!_primaryPage] == -1) {
        return _targetPage[_primaryPage];
    } else {
        return _targetPage[0];
    }
}
int PageConductor::getPageTwo()
{
    QMutexLocker locker(&_mutex);

    if (_targetPage[!_primaryPage] == -1) {
        return -1;
    } else {
        return _targetPage[1];
    }
}

int PageConductor::getPairedPage(int originalPage)
{
    QMutexLocker locker(&_mutex);

    int pairedPage;

    // There are two cases: either the page is in the same segment as the current or it isn't. If it is, use the current offset, otherwise, use the offset based on the segment boundaries.

    bool displayingTwo = _targetPage[!_primaryPage] != -1;
    bool sameSegment = displayingTwo && _pages[_targetPage[0]].prevDouble == _pages[originalPage].prevDouble;

    /// @todo THERE'S STILL SINGLE PAGES AT THE EDGES OF SEGMENTS!!

    if (sameSegment) {
        // Use the same offset as the current pages
        // See if the original page lines up the same even/odd as the first target page
        bool currentEven = _targetPage[0] % 2;
        bool originalEven = originalPage % 2;

        // Guess at the paired page
        if (currentEven == originalEven) {
            // The original page is the first of the pair
            pairedPage = originalPage + 1;
        } else {
            // The original page is the second of the pair
            pairedPage = originalPage - 1;
        }

    } else {
        // Choose the offset based on segment boundaries

        // Set the upper and lower bounds
        int currentUpper, currentLower;
        if (_targetPage[!_primaryPage] == -1) {
            currentUpper = currentLower = _targetPage[_primaryPage];
        } else {
            currentLower = _targetPage[0];
            currentUpper = _targetPage[1];
        }
        bool forward = originalPage > currentUpper;
        bool backward = originalPage < currentLower;

        if (forward) {
            int doublePage = _pages[originalPage].prevDouble;
            Q_ASSERT(doublePage != -1);
            int firstPageInSegment = doublePage + 1;
            bool firstEven = firstPageInSegment % 2;
            bool originalEven = originalPage % 2;

            // Guess at the paired page
            if (firstEven == originalEven) {
                // The original page is the first of the pair
                pairedPage = originalPage + 1;
            } else {
                // The original page is the second of the pair
                pairedPage = originalPage - 1;
            }
        } else {
            Q_ASSERT(backward);

            int doublePage = _pages[originalPage].nextDouble;
            Q_ASSERT(doublePage != -1);
            int lastPageInSegment = doublePage + 1;
            bool lastEven = lastPageInSegment % 2;
            bool originalEven = originalPage % 2;

            // Guess at the paired page
            if (lastEven == originalEven) {
                // The original page is the second of the pair
                pairedPage = originalPage - 1;
            } else {
                // The original page is the first of the pair
                pairedPage = originalPage + 1;
            }
        }
    }

    // Refine the guess: make sure the paired page isn't out of bounds or double
    if (pairedPage < -1 || pairedPage >= _numPages || _pages[pairedPage].prevDouble == pairedPage) {
        pairedPage = -1;
    }

    return pairedPage;
}

void PageConductor::turnPageForward()
{
    QMutexLocker locker(&_mutex);

    // Choose new target pages
    if (_twoPageMode) {
        _primaryPage = 0;
        if (_targetPage[1] != -1) {
            _targetPage[0] = _targetPage[1] + 1;
        } else {
            _targetPage[0]++;
        }
        _targetPage[1] = _targetPage[0] + 1;

    } else {
        _targetPage[0]++;
    }

    // Change the pages
    finishTurningPage();
}

void PageConductor::turnPageBackward()
{
    QMutexLocker locker(&_mutex);

    // Choose new target pages
    if (_twoPageMode) {
        _primaryPage = 1;
        if (_targetPage[0] != -1) {
            _targetPage[1] = _targetPage[0] - 1;
        } else {
            _targetPage[1]--;
        }
        _targetPage[0] = _targetPage[1] - 1;

    } else {
        _targetPage[0]--;
    }

    // Change the pages
    finishTurningPage();
}

void PageConductor::turnPageForwardOnePage()
{
    QMutexLocker locker(&_mutex);

    // Choose new target pages
    if (_twoPageMode) {
        _primaryPage = 0;
        if (_targetPage[0] == -1) {
            _targetPage[0] = _targetPage[1] + 1;
        } else {
            _targetPage[0]++;
        }
        _targetPage[1] = _targetPage[0] + 1;

    } else {
        _targetPage[0]++;
    }

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void PageConductor::turnPageToStart()
{
    QMutexLocker locker(&_mutex);

    // Choose new target pages
    if (_twoPageMode) {
        _primaryPage = 0;
        _targetPage[0] = 0;
        _targetPage[1] = 1;

    } else {
        _targetPage[0] = 0;
    }

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void PageConductor::turnPageToEnd()
{
    QMutexLocker locker(&_mutex);

    // Choose new target pages
    if (_twoPageMode) {
        _primaryPage = 1;
        _targetPage[1] = _numPages - 1;
        _targetPage[0] = _numPages - 2;

    } else {
        _targetPage[0] = _numPages - 1;
    }

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void PageConductor::finishTurningPage()
{
    // Check pages are within bounds
    Q_ASSERT(_targetPage[_primaryPage] >= 0 && _targetPage[_primaryPage] < _numPages);

    if (_primaryPage == 0) {
        if (_targetPage[1] >= _numPages) {
            _targetPage[1] = -1;
        }
    } else {
        if (_targetPage[0] < 0) {
            _targetPage[0] = -1;
        }
    }

    // Check that secondary page isn't wide
    if (_targetPage[!_primaryPage] != -1 && _pages[_targetPage[!_primaryPage]].prevDouble == _targetPage[!_primaryPage]) {
        _targetPage[!_primaryPage] = -1;
    }

    //kDebug()<<_targetPage[0]<<","<<_targetPage[1];

    Q_ASSERT(_targetPage[0] >= -1 && _targetPage[0] < _numPages);
    Q_ASSERT(_targetPage[1] >= -1 && _targetPage[1] < _numPages);
}

bool PageConductor::atStart()
{
    QMutexLocker locker(&_mutex);

    return _targetPage[0] == 0 || _targetPage[1] == 0;
}

bool PageConductor::atEnd()
{
    QMutexLocker locker(&_mutex);

    return _targetPage[0] == (_numPages-1) || _targetPage[1] == (_numPages-1);
}

#include "pageconductor.moc"
