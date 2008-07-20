#include "book.h"

#include <KDebug>

#include "page.h"

Book::Book(QObject *parent)
    : QObject(parent)
{
    _twoPageMode = false;
    _lister = NULL;
    _forwardEnabled = false;
    _backwardEnabled = false;
}
Book::~Book()
{
}

void Book::open(const Source &source)
{
    // Start the listing (wait for the signal when it finishes)
    Q_ASSERT(_lister == NULL);
    _lister = source.createLister();
    connect(_lister, SIGNAL(doneListing(const vector<FileInfo> &)), this, SLOT(doneListing(const vector<FileInfo> &)));
    _lister->beginListing();
}

void Book::doneListing(const vector<FileInfo> &files)
{
    // Create the list of pages
    _pages.clear();

    _numPages = files.size();
    Q_ASSERT(_numPages > 0);

    for (int i=0; i<_numPages; i++) {
        _pages.push_back(Page(files[i]));
    }

    // Free the lister
    Q_ASSERT(_lister != NULL);
    delete _lister;
    _lister = NULL;

    // Choose the starting pages
    _primaryPage = 0;
    _targetPage[0] = 0;
    _targetPage[1] = 1;

    finishTurningPage();
}

int Book::getPageOne()
{
    if (_targetPage[!_primaryPage] == -1) {
        return _targetPage[_primaryPage];
    } else {
        return _targetPage[0];
    }
}
int Book::getPageTwo()
{
    if (_targetPage[!_primaryPage] == -1) {
        return -1;
    } else {
        return _targetPage[1];
    }
}

QString getFilename(int pageNumber) const
{
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);
    return _pages[pageNumber].filename;
}

int getFilesize(int pageNumber) const
{
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);
    return _pages[pageNumber].filesize;
}

QSize getImageSize(int pageNumber) const
{
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);
    return _pages[pageNumber].imageSize;
}

int Book::getPairedPage(int originalPage) const
{
    Q_ASSERT(originalPage >= 0 && originalPage < _numPages);

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

void setPageDouble(int pageNumber)
{
    /// @todo setPageDouble
}

void setPageNotDouble(int pageNumber)
{
    /// @todo setPageDouble
}

void Book::turnPageForward()
{
    // Choose new target pages
    _primaryPage = 0;
    if (_targetPage[1] != -1) {
        _targetPage[0] = _targetPage[1] + 1;
    } else {
        _targetPage[0]++;
    }
    _targetPage[1] = _targetPage[0] + 1;

    // Change the pages
    finishTurningPage();
}

void Book::turnPageBackward()
{
    // Choose new target pages
    _primaryPage = 1;
    if (_targetPage[0] != -1) {
        _targetPage[1] = _targetPage[0] - 1;
    } else {
        _targetPage[1]--;
    }
    _targetPage[0] = _targetPage[1] - 1;

    // Change the pages
    finishTurningPage();
}

void Book::turnPageForwardOnePage()
{
    // Choose new target pages
    _primaryPage = 0;
    if (_targetPage[0] == -1) {
        _targetPage[0] = _targetPage[1] + 1;
    } else {
        _targetPage[0]++;
    }
    _targetPage[1] = _targetPage[0] + 1;

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void Book::turnPageToStart()
{
    // Choose new target pages
    _primaryPage = 0;
    _targetPage[0] = 0;
    _targetPage[1] = 1;

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void Book::turnPageToEnd()
{
    // Choose new target pages
    _primaryPage = 1;
    _targetPage[1] = _numPages - 1;
    _targetPage[0] = _numPages - 2;

    // Change the pages
    finishTurningPage();

    // Page order changed
    emit pagePairsChanged(0, _numPages - 1);
}

void Book::finishTurningPage()
{
    // Check pages are within bounds
    Q_ASSERT(_targetPage[_primaryPage] >= 0 && _targetPage[_primaryPage] < _numPages);

    if (_twoPageMode) {
        if (_primaryPage == 0) {
            if (__targetPage[1] >= _numPages) {
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
    } else {
        _targetPage[!_primaryPage] = -1;
    }

    Q_ASSERT(_targetPage[0] >= -1 && _targetPage[0] < _numPages);
    Q_ASSERT(_targetPage[1] >= -1 && _targetPage[1] < _numPages);

    // Check if forwards or backwards has been enabled/disabled
    bool forwardPreviouslyEnabled = _forwardEnabled;
    bool backwardPreviouslyEnabled = _backwardEnabled;

    _forwardEnabled = _targetPage[0] != (_numPages-1) && _targetPage[1] != (_numPages-1);
    _backwardEnabled = _targetPage[0] != 0 && _targetPage[1] != 0;

    if (_forwardEnabled != forwardPreviouslyEnabled) {
        emit forwardEnabled(_forwardEnabled);
    }
    if (_backwardEnabled != backwardPreviouslyEnabled) {
        emit backwardEnabled(_backwardEnabled);
    }

    //kDebug()<<_targetPage[0]<<","<<_targetPage[1];

    emit pageTurned();
}


#include "book.moc"
