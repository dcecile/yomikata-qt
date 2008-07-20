#include "planner.h"

#include <QMutexLocker>
#include <KDebug>

#include "pageconductor.h"

Planner::Planner(PageConductor &pageConductor, QObject *parent)
    : QObject(parent), _pageConductor(pageConductor), _numPages(0)
{
    _mangaMode = true;

    // Connect to the page conductor
    connect(&_pageConductor, SIGNAL(pagePairsChanged(int, int)), this, SLOT(pagePairsChanged(int, int)));
}

Planner::~Planner()
{
}

void Planner::initialize(int numPages)
{
    QMutexLocker locker(&_mutex);

    _fullSizes.clear();
    _targetRects.clear();
    _numPages = numPages;
    _fullSizes.resize(_numPages);
    _targetRects.resize(_numPages);
}

void Planner::areaResized(const QSize &newSize)
{
    QMutexLocker locker(&_mutex);

    // Set the area
    _areaSize = newSize;

    // Discard cached target rects
    fill(_targetRects.begin(), _targetRects.end(), QRect());
}

QRect Planner::plan(int pageNumber)
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(_areaSize.isValid());
    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);

    // Check if the result is cached
    if (_targetRects[pageNumber].isValid()) {
        //kDebug()<<"page"<<pageNumber<<"cached"<<_targetRects[pageNumber];
        return _targetRects[pageNumber];
    }

    // Get this page's size
    QSize pageSize = _fullSizes[pageNumber];
    if (!pageSize.isValid()) {
        pageSize.setWidth(DEFAULT_WIDTH);
        pageSize.setHeight(DEFAULT_HEIGHT);
    }

    // Get the paired page
    int pairedPageNumber = _pageConductor.getPairedPage(pageNumber);
    Q_ASSERT(pageNumber != pairedPageNumber);

    // If this page is solitary
    if (pairedPageNumber == -1) {

        // Cache the planned rect
        _targetRects[pageNumber] = plan(pageSize);

        // Return it
        return _targetRects[pageNumber];
    }

    // Get the paired page's size
    QSize pairedPageSize = _fullSizes[pairedPageNumber];
    if (!pairedPageSize.isValid()) {
        pairedPageSize.setWidth(DEFAULT_WIDTH);
        pairedPageSize.setHeight(DEFAULT_HEIGHT);
    }

    QSize tempA, tempB;
    QRect targetA, targetB;

    // Check which order the pages are
    bool pairedOnLeft = (pageNumber < pairedPageNumber && _mangaMode) || (pageNumber > pairedPageNumber && !_mangaMode);
    if (pairedOnLeft) {
        tempA = pairedPageSize;
        tempB = pageSize;
    } else {
        tempA = pageSize;
        tempB = pairedPageSize;
    }

    // Guess the sizes if needed
    /// @todo Choose default width and height based on mode dimensions
    if (!tempA.isValid()) {
        tempA.setWidth(DEFAULT_WIDTH);
        tempA.setHeight(DEFAULT_HEIGHT);
    }
    if (!tempB.isValid()) {
        tempB.setWidth(DEFAULT_WIDTH);
        tempB.setHeight(DEFAULT_HEIGHT);
    }

    // Scale the sizes to eachother so their heights match
    //kDebug()<<tempA<<tempB;
    convertToLargestHeight(&tempA, &tempB);
    //kDebug()<<tempA<<tempB;

    // Scale the combined rect and centre it
    QRect combinedTarget = plan(QSize(tempA.width() + tempB.width(), tempA.height()));

    // If the two starting sizes are the same, round the combined width up
    if (tempA == tempB) {
        combinedTarget.setWidth(combinedTarget.width() + (combinedTarget.width() % 2));
    }

    //kDebug()<<"area"<<_areaSize;
    //kDebug()<<"combined"<<combinedTarget;

    // Calculate the target sizes
    targetA.setWidth( int (
            double(combinedTarget.width()) * double(tempA.width())
            / double(tempA.width() + tempB.width()) + 0.5 ));
    targetA.setHeight(combinedTarget.height());
    targetB.setWidth(combinedTarget.width() - targetA.width());
    targetB.setHeight(combinedTarget.height());

    // Position the target rectangles
    targetA.moveTo(combinedTarget.topLeft());
    targetB.moveTo(combinedTarget.topLeft() + QPoint(targetA.width(), 0));

    //kDebug()<<targetA<<targetB;

    // Store the results
    if (pairedOnLeft) {
        _targetRects[pageNumber] = targetB;
        _targetRects[pairedPageNumber] = targetA;
    } else {
        _targetRects[pageNumber] = targetA;
        _targetRects[pairedPageNumber] = targetB;
    }

    // Return the target rect
    return _targetRects[pageNumber];
}

QRect Planner::plan(QSize fullSize)
{
    // Guess the size if needed
    if (!fullSize.isValid()) {
        fullSize.setWidth(DEFAULT_WIDTH);
        fullSize.setHeight(DEFAULT_HEIGHT);
    }
    fullSize.scale(_areaSize, Qt::KeepAspectRatio);

    // Centre the image in the full area
    QRect targetRect;
    targetRect.setSize(fullSize);
    centreRect(&targetRect);

    // Return the rect
    return targetRect;
}

void Planner::centreRect(QRect *target)
{
    // Centre the rectangle, rounding to the left and up
    //kDebug()<<_areaSize.width()<<target->width();
    //kDebug()<<double(_areaSize.width() - target->width()) / 2.0;
    target->moveTo(
            int ( double(_areaSize.width() - target->width()) / 2.0 ),
            int ( double(_areaSize.height() - target->height()) / 2.0 )
            );
}

bool Planner::isWide(int pageNum)
{
    const QSize &size = _fullSizes[pageNum];
    if (size.isValid()) {
        return (double(size.width()) / double(size.height())) > WIDE_PAGE_RATIO;
    } else {
        return false;
    }
}

void Planner::convertToLargestHeight(QSize *sizeA, QSize *sizeB)
{
    // If they already have the same height, no need to do anything
    if (sizeA->height() == sizeB->height()) {
        return;
    }

    // Choose which one is the biggest (tallest)
    QSize *big, *small;
    if (sizeA->height() > sizeB->height()) {
        big = sizeA;
        small = sizeB;
    } else {
        big = sizeB;
        small = sizeA;
    }


    // Scale the smaller size up to the bigger one

    double bigWideness = double(big->width()) / double(big->height());
    double smallWideness = double(small->width()) / double(small->height());

    // If the big one is wider, the small one will fit in it
    if (bigWideness > smallWideness) {
        small->scale(*big, Qt::KeepAspectRatio);

    // If the small one is wider, it will pop out of the big one
    } else {
        small->scale(*big, Qt::KeepAspectRatioByExpanding);
    }

    // The heights should now be equal
    Q_ASSERT(sizeA->height() == sizeB->height());
}

bool Planner::isFullSizeKnown(int pageNumber)
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);
    return _fullSizes[pageNumber].isValid();
}
void Planner::setFullSize(int pageNumber, QSize fullSize)
{
    QMutexLocker locker(&_mutex);

    Q_ASSERT(pageNumber >= 0 && pageNumber < _numPages);

    // Set the full size
    _fullSizes[pageNumber] = fullSize;

    // Discard this page's cached target rect
    _targetRects[pageNumber] = QRect();

    // Get the paired page
    int pairedPageNumber = _pageConductor.getPairedPage(pageNumber);
    Q_ASSERT(pairedPageNumber >= -1 && pairedPageNumber < _numPages);

    // If this page isn't solitary
    if (pairedPageNumber != -1) {
        // Discard the other page's cached target rect
        _targetRects[pairedPageNumber] = QRect();
    }

    // Notify if the current pages need replanning
    if (pageNumber == _pageConductor.getPageOne() || pageNumber == _pageConductor.getPageTwo()) {
        emit currentPagesReplanned();
    }

    // Notify that page sizes have been reset
    if (pairedPageNumber == -1) {
        emit plannedSizesReset(pageNumber, pageNumber);
    } else if (pairedPageNumber > pageNumber) {
        emit plannedSizesReset(pageNumber, pairedPageNumber);
    } else {
        emit plannedSizesReset(pairedPageNumber, pageNumber);
    }
}

void Planner::pagePairsChanged(int lowerPageNumber, int upperPageNumber)
{
    kDebug()<<lowerPageNumber<<upperPageNumber<<_numPages;
    Q_ASSERT(lowerPageNumber >= 0 && lowerPageNumber < _numPages);
    Q_ASSERT(upperPageNumber >= 0 && upperPageNumber < _numPages);
    Q_ASSERT(lowerPageNumber <= upperPageNumber);

    for (int i = lowerPageNumber; i <= upperPageNumber; i++) {
        _targetRects[i] = QRect();
    }

    emit plannedSizesReset(lowerPageNumber, upperPageNumber);
}

