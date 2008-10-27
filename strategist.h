#ifndef STRATEGIST_H
#define STRATEGIST_H

#include <QObject>

#include <QRect>

#include <vector>

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

    QRect pageLayout(int index);

    bool isFullPageSizeKnown(int index);
    void setFullPageSize(int index, QSize size);

    void setViewport(const QSize &size);

signals:
    void recievedFullPageSize(int index);

private:
    static const int DEFAULT_WIDTH = 5;
    static const int DEFAULT_HEIGHT = 7;
    static const double DUAL_PAGE_RATIO = 1.0;

private:
    void layOutPages(QRect *rect0, QRect *rect1, QSize fullSize0, QSize fullSize1);
    void convertToLargestHeight(QSize *size0, QSize *size1);
    QRect layOutPage(QSize fullSize);

private:
    QMutex &_lock;
    Book &_book;
    QSize _viewport;
    int _numPages;
    vector<QSize> _fullSizes;
};

#endif
