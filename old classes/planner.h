#ifndef PLANNER_H
#define PLANNER_H

#include <QObject>
#include <QRect>
#include <QMutex>

#include <vector>

class PageConductor;

using std::vector;

/**
 * @defgroup mod_planner Planner module
 * Lays out the pages on the presentaion area, chooses good sizes
 */

/**
 * @ingroup mod_userinterface
 * @brief Lays out the pages on the presentaion area
 */
class Planner : public QObject
{
    Q_OBJECT

public:
    Planner(PageConductor &pageConductor, QObject *parent);

    void areaResized(const QSize &newSize);

    void initialize(int numPages);

    QRect plan(int pageNumber);

    bool isFullSizeKnown(int pageNumber);
    void setFullSize(int pageNumber, QSize fullSize);

    ~Planner();

signals:
    void currentPagesReplanned();
    void plannedSizesReset(int lowerPageNumber, int upperPageNumber);

private slots:
    void pagePairsChanged(int lowerPageNumber, int upperPageNumber);

private:
    void convertToLargestHeight(QSize *sizeA, QSize *sizeB);

private:
    static const int DEFAULT_WIDTH = 5;
    static const int DEFAULT_HEIGHT = 7;
    static const double WIDE_PAGE_RATIO = 1.0;

private:
    QRect plan(QSize fullSize);
    void centreRect(QRect *target);
    bool isWide(int pageNum);

private:
    PageConductor &_pageConductor;

    QMutex _mutex;

    bool _mangaMode;
    QSize _areaSize;
    vector<QSize> _fullSizes;
    vector<QRect> _targetRects;
    int _numPages;
};

#endif
