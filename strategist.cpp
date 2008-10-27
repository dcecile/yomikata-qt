#include "strategist.h"

#include <QMutex>

#include "book.h"

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
            fullSize0 = QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        }

        // Calculate layout for both pages
        QRect rect0;
        QRect rect1;
        layOutPages(&rect0, &rect1, fullSize0, fullSize1);

        // Return the matching page
        if (page0 == index)
        {
            return rect0;
        }
        else
        {
            return rect1;
        }
    }
    // One page
    else
    {
        // Calculate layout for the page
        return layOutPage(fullSize0);
    }
}

void Strategist::layOutPages(QRect *rect0, QRect *rect1, QSize fullSize0, QSize fullSize1)
{
    // Scale the sizes to eachother so their heights match
    convertToLargestHeight(&fullSize0, &fullSize1);

    // Scale the combined rect and centre it
    QRect combinedTarget = layOutPage(QSize(fullSize0.width() + fullSize1.width(), fullSize0.height()));

    // If the two starting sizes (scaled to match heights) are the same, round
    //   the combined width down
    if (fullSize0 == fullSize1)
    {
        combinedTarget.setWidth(combinedTarget.width() - (combinedTarget.width() % 2));
    }

    // Calculate the target sizes
    rect0->setWidth(int(
            double(combinedTarget.width()) * double(fullSize0.width())
            / double(fullSize0.width() + fullSize1.width()) + 0.5));
    rect0->setHeight(combinedTarget.height());
    rect1->setWidth(combinedTarget.width() - rect0->width());
    rect1->setHeight(combinedTarget.height());

    // Position the target rectangles
    rect0->moveTo(combinedTarget.topLeft());
    rect1->moveTo(combinedTarget.topLeft() + QPoint(rect0->width(), 0));
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

QRect Strategist::layOutPage(QSize fullSize)
{
    // Fit into the viewport
    fullSize.scale(_viewport, Qt::KeepAspectRatio);

    // Centre the image in the full area
    QPoint topLeft(int(double(_viewport.width() - fullSize.width()) / 2.0 + 0.5),
                   int(double(_viewport.height() - fullSize.height()) / 2.0 + 0.5));

    // Return the rect
    return QRect(topLeft, fullSize);
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

void Strategist::setViewport(const QSize &size)
{
    QMutexLocker locker(&_lock);
    _viewport = size;
}

#include "strategist.moc"
