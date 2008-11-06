#include "steward.h"

#include <QLabel>

#include "debug.h"
#include "book.h"
#include "indexer.h"
#include "strategist.h"
#include "artificer.h"
#include "projector.h"
#include "debugwidget.h"

Steward::Steward(QObject *parent)
    : QObject(parent),
    _lock(QMutex::Recursive),
    _book(*new Book(_lock, this)),
    _indexer(*new Indexer(this)),
    _strategist(*new Strategist(_book, _lock, this)),
    _artificer(*new Artificer(_indexer, _strategist, this)),
    _projector(*new Projector(NULL)),
    _debugWidget(new DebugWidget(_book, NULL))
{
    // Connect
    connect(&_book, SIGNAL(dualCausedPageChange()), SLOT(dualCausedPageChange()));
    connect(&_indexer, SIGNAL(built()), SLOT(indexerBuilt()));
    connect(&_strategist, SIGNAL(recievedFullPageSize(int)), SLOT(recievedFullPageSize(int)));
    connect(&_artificer, SIGNAL(pageDecoded(int, QPixmap)), SLOT(decodeDone(int, QPixmap)));
    connect(&_projector, SIGNAL(resized(const QSize&)), SLOT(viewportResized(const QSize&)));

    _buildingIndexer = false;
}

Steward::~Steward()
{
    // Stop the threads first
    delete &_artificer;
}

QWidget *Steward::projector()
{
    return &_projector;
}

QWidget *Steward::debugWidget()
{
    return _debugWidget;
}

void Steward::reset(const QString &filename)
{
    // Stop decodes
    _artificer.reset();

    // Pretend two page book, show loading
    _book.reset(2);
    _strategist.reset();
    _projector.showLoading0(_strategist.pageLayout(0));
    _projector.showLoading1(_strategist.pageLayout(1));

    // Start the indexerer
    _indexer.reset(filename);

    // And wait
    _buildingIndexer = true;
}

void Steward::indexerBuilt()
{
    _buildingIndexer = false;

    // Don't do anything with an empty book
    if (_indexer.numPages() == 0)
    {
        _projector.showBlank();
        return;
    }

    // New book size
    _book.reset(_indexer.numPages());
    _strategist.reset();

    // Show the first two pages
    pageChanged();
}

void Steward::next()
{
    if (_book.isNextEnabled())
    {
        _book.next();
        pageChanged();
    }
}

void Steward::previous()
{
    if (_book.isPreviousEnabled())
    {
        _book.previous();
        pageChanged();
    }
}

void Steward::shiftNext()
{
    if (_book.isNextEnabled())
    {
        _book.shiftNext();
        pageChanged();
    }
}

void Steward::pageChanged()
{
    // Show the current page(s)
    loadPages();

    // Start scroll to the start of the pages
    _projector.pagesChanged();
}

void Steward::loadPages()
{
    // Show the current page(s)
    int current0 = _book.page0();
    int current1 = _book.page1();

    _projector.showBlank();

    _projector.showLoading0(_strategist.pageLayout(current0));

    if (current1 >= 0)
    {
        _projector.showLoading1(_strategist.pageLayout(current1));
    }

    _artificer.decodePages(current0, current1);
}

/**
 * @todo Manage request queue better for failed decodes
 *   (know if each page is correct)
 */
void Steward::decodeDone(int index, QPixmap page)
{
    // Display the page if needed
    int current0 = _book.page0();
    int current1 = _book.page1();

    if (index == current0)
    {
        QRect layout = _strategist.pageLayout(index);

        // Show the page if it's correct
        if (page.size() == layout.size())
        {
            _projector.showPage0(layout, page);
        }
        // Or try decoding again, if needed
        else
        {
            debug()<<"Wrong size"<<index;
            _artificer.decodePages(current0, current1);
        }
    }
    else if (index == current1)
    {
        QRect layout = _strategist.pageLayout(index);

        // Show the page if it's correct
        if (page.size() == layout.size())
        {
            _projector.showPage1(layout, page);
        }
        // Or try decoding again, if needed
        else
        {
            debug()<<"Wrong size"<<index;
            _artificer.decodePages(current0, current1);
        }
    }
}

void Steward::viewportResized(const QSize &size)
{
    debug()<<"Resized"<<size;

    // Notify the strategist
    _strategist.setViewport(size);

    // Check if the book is being opened
    if (_buildingIndexer)
    {
        // Resize the loading dialogs
        _projector.showLoading0(_strategist.pageLayout(0));
        _projector.showLoading1(_strategist.pageLayout(1));
    }
    // Check that there is a book
    else if (_book.numPages() > 0)
    {
        // Reload the pages
        loadPages();
    }
}

/**
 * @todo Do something if the sizes are wrong
 */
void Steward::recievedFullPageSize(int index)
{
    // Update the projector's display if a current page was affected
    int current0 = _book.page0();
    int current1 = _book.page1();

    if (current0 == index || current1 == index)
    {
        _projector.updatePosition0(_strategist.pageLayout(current0));

        if (current1 >= 0)
        {
            _projector.updatePosition1(_strategist.pageLayout(current1));
        }
    }
}

void Steward::dualCausedPageChange()
{
    // Reload the current pages
    loadPages();
}

#include "steward.moc"
