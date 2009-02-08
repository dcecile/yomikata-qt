#include "strategist.h"

#include <QMutex>

#include <algorithm>

#include "debug.h"
#include "book.h"

using std::max;

const int Strategist::DEFAULT_WIDTH = 934;
const int Strategist::DEFAULT_HEIGHT = 1500;
const double Strategist::DUAL_PAGE_RATIO = 1.0;

Strategist::Strategist(Book &book, QMutex &lock, QObject *parent)
    : QObject(parent), _lock(lock), _book(book)
{
    _numPages = 0;
}

Strategist::~Strategist()
{
}

void Strategist::reset()
{
    QMutexLocker locker(&_lock);

    _numPages = _book.numPages();
    _fullSizes.clear();
    _fullSizes.resize(_numPages);
}

DisplayMetrics Strategist::pageLayout()
{
    QMutexLocker locker(&_lock);

    // Invalid viewport size means the widget is not set up
    if (!_viewport.isValid())
    {
        return DisplayMetrics();
    }

    return pageLayout(_book.page0(), _book.page1());
}

QRect Strategist::pageLayout(int index)
{
    QMutexLocker locker(&_lock);

    Q_ASSERT(index >= 0 && index < _numPages);

    // Invalid viewport size means the widget is not set up
    if (!_viewport.isValid())
    {
        return QRect(0, 0, 0, 0);
    }

    // Find the page(s) that will be displayed together
    int page0;
    int page1;

    // Target page is second
    if (_book.pairedPageOffset(index) == 1)
    {
        page1 = index;
        page0 = index + 1;
    }
    // Target page is first or solitary
    else
    {
        page0 = index;
        page1 = _book.pairedPage(index);
    }

    DisplayMetrics displayMetrics = pageLayout(page0, page1);

    if (page0 == index)
    {
        return displayMetrics.pages[0];
    }
    else
    {
        return displayMetrics.pages[1];
    }
}

DisplayMetrics Strategist::pageLayout(int page0, int page1)
{
    // Retrieve the full size
    QSize fullSize0 = _fullSizes[page0];

    // Check if it was found
    if (!fullSize0.isValid())
    {
        fullSize0 = QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }

    // Two pages
    if (page1 >= 0)
    {
        // Retrieve the full size
        QSize fullSize1 = _fullSizes[page1];

        // Check if it was found
        if (!fullSize1.isValid())
        {
            fullSize1 = QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        }

        // Calculate layout for both pages
        return layOutPages(fullSize0, fullSize1);
    }
    // One page
    else
    {
        // Calculate layout for the page
        return layOutPage(fullSize0);
    }
}

DisplayMetrics Strategist::layOutPages(QSize fullSize0, QSize fullSize1)
{
    // Scale the sizes to eachother so their heights match
    convertToLargestHeight(&fullSize0, &fullSize1);

    // Scale the combined rect and centre it
    DisplayMetrics displayMetrics = layOutPage(QSize(fullSize0.width() + fullSize1.width(), fullSize0.height()));
    QRect combinedTarget = displayMetrics.pages[0];

    // If the two starting sizes (scaled to match heights) are the same, round
    //   the combined width down
    if (fullSize0 == fullSize1)
    {
        int rounding = combinedTarget.width() % 2;
        combinedTarget.setWidth(combinedTarget.width() - rounding);
        displayMetrics.slack.setWidth(max(0, displayMetrics.slack.width() - rounding));
    }

    // Calculate the target sizes
    displayMetrics.pages[0].setWidth(int(
            double(combinedTarget.width()) * double(fullSize0.width())
            / double(fullSize0.width() + fullSize1.width()) + 0.5));
    displayMetrics.pages[0].setHeight(combinedTarget.height());
    displayMetrics.pages[1].setWidth(combinedTarget.width() - displayMetrics.pages[0].width());
    displayMetrics.pages[1].setHeight(combinedTarget.height());

    // Position the target rectangles
    displayMetrics.pages[0].moveTo(combinedTarget.topLeft());
    displayMetrics.pages[1].moveTo(combinedTarget.topLeft() + QPoint(displayMetrics.pages[0].width(), 0));

    return displayMetrics;
}

void Strategist::convertToLargestHeight(QSize *size0, QSize *size1)
{
    // If they already have the same height, no need to do anything
    if (size0->height() == size1->height())
    {
        return;
    }

    // Choose which one is the biggest (tallest)
    QSize *big, *small;
    if (size0->height() > size1->height())
    {
        big = size0;
        small = size1;
    }
    else
    {
        big = size1;
        small = size0;
    }

    // Scale the smaller size up to the bigger one
    double bigWideness = double(big->width()) / double(big->height());
    double smallWideness = double(small->width()) / double(small->height());

    // If the big one is wider, the small one will fit in it
    if (bigWideness > smallWideness)
    {
        small->scale(*big, Qt::KeepAspectRatio);
    }
    // If the small one is wider, it will pop out of the big one
    else
    {
        small->scale(*big, Qt::KeepAspectRatioByExpanding);
    }

    // The heights should now be equal
    Q_ASSERT(size0->height() == size1->height());
}

DisplayMetrics Strategist::layOutPage(QSize fullSize)
{
    DisplayMetrics displayMetrics;

    // Fit into the viewport
    fullSize.scale(_viewport, Qt::KeepAspectRatio);

    // Centre or allow for scrolling as needed
    QPoint topLeft;

    // Set left
    if (fullSize.width() > _visibleSize.width())
    {
        // Push left (too big to centre)
        topLeft.setX(0);
    }
    else
    {
        // Centre width
        topLeft.setX(int(double(_visibleSize.width() - fullSize.width()) / 2.0 + 0.5));
    }

    // Set top
    if (fullSize.height() > _visibleSize.height())
    {
        // Push up (too big to centre)
        topLeft.setY(0);
    }
    else
    {
        // Centre height
        topLeft.setY(int(double(_visibleSize.height() - fullSize.height()) / 2.0 + 0.5));
    }

    // Set the rect
    displayMetrics.pages[0] = QRect(topLeft, fullSize);

    // Measure the slack
    displayMetrics.slack = QSize(
        max(0, fullSize.width() - _visibleSize.width()),
        max(0, fullSize.height() - _visibleSize.height()));

    return displayMetrics;
}

bool Strategist::isFullPageSizeKnown(int index)
{
    QMutexLocker locker(&_lock);

    Q_ASSERT(index >= 0 && index < _numPages);

    return _fullSizes[index].isValid();
}

/**
 * @todo Handle double-message for current and dual
 */
void Strategist::setFullPageSize(int index, QSize size)
{
    QMutexLocker locker(&_lock);

    Q_ASSERT(index >= 0 && index < _numPages);

    _fullSizes[index] = size;

    // Check for dual pages
    if (double(size.width()) / double(size.height()) >= DUAL_PAGE_RATIO)
    {
        _book.setDual(index);
    }

    // Layouts and caches may need to change
    emit recievedFullPageSize(index);
}

void Strategist::setViewport(const QSize &fullSize, const QSize &viewSize)
{
    QMutexLocker locker(&_lock);
    _viewport = fullSize;
    _visibleSize = viewSize;
}

#include "strategist.moc"
