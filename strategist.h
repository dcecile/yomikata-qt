#ifndef STRATEGIST_H
#define STRATEGIST_H

#include <QObject>

#include <QRect>

#include <vector>

#include "displaymetrics.h"

class QMutex;

class Book;

using std::vector;

class Strategist : public QObject
{
    Q_OBJECT

public:
    Strategist(Book &book, QMutex &lock, QObject *parent = NULL);
    ~Strategist();

    void reset();

    DisplayMetrics pageLayout();
    QRect pageLayout(int index);

    bool isFullPageSizeKnown(int index);
    void setFullPageSize(int index, QSize size);

    void setViewport(const QSize &fullSize, const QSize &viewSize);

signals:
    void recievedFullPageSize(int index);

private:
    static const int DEFAULT_WIDTH;
    static const int DEFAULT_HEIGHT;
    static const double DUAL_PAGE_RATIO;

private:
    DisplayMetrics pageLayout(int page0, int page1);
    DisplayMetrics layOutPages(QSize fullSize0, QSize fullSize1);
    void convertToLargestHeight(QSize *size0, QSize *size1);
    DisplayMetrics layOutPage(QSize fullSize);

private:
    QMutex &_lock;
    Book &_book;
    QSize _viewport;
    QSize _visibleSize;
    int _numPages;
    vector<QSize> _fullSizes;
};

#endif
