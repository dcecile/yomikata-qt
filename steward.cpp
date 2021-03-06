#include "steward.h"

#include <QLabel>

#include "debug.h"
#include "book.h"
#include "archive.h"
#include "indexer.h"
#include "strategist.h"
#include "artificer.h"
#include "projector.h"

Steward::Steward(QObject *parent)
    : QObject(parent),
    _book(*new Book(this)),
    _archive(*new Archive(this)),
    _indexer(*new Indexer(_archive, this)),
    _strategist(*new Strategist(_book, this)),
    _artificer(*new Artificer(_archive, _indexer, _strategist, this)),
    _projector(*new Projector(NULL))
{
    // Connect
    connect(&_book, SIGNAL(dualCausedPageChange()), SLOT(dualCausedPageChange()));
    connect(&_indexer, SIGNAL(built()), SLOT(indexerBuilt()));
    connect(&_strategist, SIGNAL(recievedFullPageSize(int)), SLOT(recievedFullPageSize(int)));
    connect(&_artificer, SIGNAL(pageDecoded(int, QPixmap)), SLOT(decodeDone(int, QPixmap)));
    connect(&_projector, SIGNAL(update()), SIGNAL(viewUpdate()));
    connect(&_projector, SIGNAL(repaint()), SIGNAL(viewRepaint()));

    _buildingIndexer = false;
}

Steward::~Steward()
{
    // Stop the threads first
    delete &_artificer;
}

void Steward::reset(const QString &filename)
{
    // Stop decodes
    _artificer.reset();

    // Pretend two page book, show loading
    _book.reset(2);
    _strategist.reset();
    _projector.clear(_strategist.pageLayout());

    // Retrieve the archive information
    _archive.reset(filename);

    // Start the indexerer
    _indexer.reset();

    // And wait
    _buildingIndexer = true;
}

void Steward::indexerBuilt()
{
    _buildingIndexer = false;

    // Don't do anything with an empty book
    if (_indexer.numPages() == 0)
    {
        _projector.clear(DisplayMetrics());
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

void Steward::setPage(int page)
{
    if (page < _book.numPages())
    {
        _book.setPage(page);
        pageChanged();
    }
}

void Steward::pageChanged()
{
    // Show the current page(s)
    loadPages();

    // Notify that the pages changed
    emit pageChanged(_book.page0(), _book.numPages());
}

void Steward::loadPages()
{
    // Display loading
    _projector.clear(_strategist.pageLayout());

    // Decode both pages
    int current0 = _book.page0();
    int current1 = _book.page1();
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
        DisplayMetrics displayMetrics = _strategist.pageLayout();

        // Show the page if it's correct
        if (page.size() == displayMetrics.pages[0].size())
        {
            //qDebug()<<"Page 0"<<displayMetrics.pages[0].topLeft();
            _projector.update(displayMetrics, page, QPixmap());
        }
        // Or try decoding again, if needed
        else
        {
            debug()<<"Wrong size"<<index<<page.size();
            _artificer.decodePages(current0, current1);
        }
    }
    else if (index == current1)
    {
        DisplayMetrics displayMetrics = _strategist.pageLayout();

        // Show the page if it's correct
        if (page.size() == displayMetrics.pages[1].size())
        {
            //qDebug()<<"Page 1"<<displayMetrics.pages[1].topLeft();
            _projector.update(displayMetrics, QPixmap(), page);
        }
        // Or try decoding again, if needed
        else
        {
            debug()<<"Wrong size"<<index;
            _artificer.decodePages(current0, current1);
        }
    }
}

/**
 * @todo More sophisticated do-nothing resize checks
 */
void Steward::setViewSize(const QSize &size)
{
    // Skip the update if nothing's changed
    if (size == _projector.viewSize())
    {
        return;
    }

    // Notify the projector
    _projector.setViewSize(size);

    // Notify the strategist
    _strategist.setViewport(_projector.fullSize(), size);

    // Check if the book is being opened
    if (_buildingIndexer)
    {
        // Resize the loading indicators
        _projector.clear(_strategist.pageLayout());
    }
    // Check that there is a book
    else if (_book.numPages() > 0)
    {
        // Reload the pages
        loadPages();
    }
}

void Steward::paintView(QPainter *painter, const QRect &updateRect)
{
    // Have the projector paint
    _projector.paint(painter, updateRect);
}

void Steward::mouseMoved(const QPointF &pos)
{
    // Tell the projector about the mouse movement
    _projector.mouseMoved(pos);
}

void Steward::resetMouse()
{
    // Tell the projector about the mouse being reset
    _projector.resetMouse();
}

/**
 * @todo Make robust
 */
void Steward::recievedFullPageSize(int index)
{
    // Update the projector's display if a current page was affected
    int current0 = _book.page0();
    int current1 = _book.page1();

    if (current0 == index || current1 == index)
    {
        bool updated = _projector.tryUpdate(_strategist.pageLayout());

        if (!updated)
        {
            debug()<<"Not updated";
            loadPages();
        }
    }
}

void Steward::dualCausedPageChange()
{
    // Reload the current pages
    loadPages();
}
