#include "book.h"

#include <KDebug>

#include "page.h"

Book::Book(QObject *parent)
    : QObject(parent), _pageConductor(this), _planner(_pageConductor, this), _cache(_planner, this)
{
    connect(&_planner, SIGNAL(currentPagesReplanned()), this, SIGNAL(currentPagesReplanned()));
}
Book::~Book()
{
}

void Book::open(const vector<FileInfo> &files)
{
    // Create the list of pages
    _pages.clear();
    _numPages = files.size();
    Q_ASSERT(_numPages > 0);
    Page *temp;
    for (int i=0; i<_numPages; i++) {
        temp = new Page(i, files[i], _planner, _cache, this);
        _pages.push_back(temp);
    }

    // Open to the first page
    _pageConductor.initialize(_numPages);

    // Initialize the planner
    _planner.initialize(_numPages);

    // Initialize the cache
    _cache.initialize(_numPages, _pageConductor.getPageOne(), _pageConductor.getPageTwo());
}

Page *Book::getPageOne()
{
    int pageNumber = _pageConductor.getPageOne();
    if (pageNumber == -1) {
        return NULL;
    } else {
        return _pages[pageNumber];
    }
}
Page *Book::getPageTwo()
{
    int pageNumber = _pageConductor.getPageTwo();
    if (pageNumber == -1) {
        return NULL;
    } else {
        return _pages[pageNumber];
    }
}

Page *Book::getPageToPrecache()
{
    int pageNumber = _cache.getPageToPrecache();
    if (pageNumber == -1) {
        return NULL;
    } else {
        return _pages[pageNumber];
    }
}

void Book::resetDisplayArea(const QSize &area)
{
    _planner.areaResized(area);
}

void Book::turnPageForward()
{
    _pageConductor.turnPageForward();
    finishTurningPage();
}

void Book::turnPageBackward()
{
    _pageConductor.turnPageBackward();
    finishTurningPage();
}

void Book::turnPageForwardOnePage()
{
    _pageConductor.turnPageForwardOnePage();
    finishTurningPage();
}

void Book::turnPageToStart()
{
    _pageConductor.turnPageToStart();
    finishTurningPage();
}

void Book::turnPageToEnd()
{
    _pageConductor.turnPageToEnd();
    finishTurningPage();
}

void Book::finishTurningPage()
{
    // Mark pages as currently open to
    // Keep the cache prioritized
    _cache.setCurrentPages(_pageConductor.getPageOne(), _pageConductor.getPageTwo());
}

bool Book::atStart()
{
    return _pageConductor.atStart();
}

bool Book::atEnd()
{
    return _pageConductor.atEnd();
}

#include "book.moc"
