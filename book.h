#ifndef BOOK_H
#define BOOK_H

#include <QObject>
#include <QList>

#include <vector>

#include "fileinfo.h"
#include "pageconductor.h"
#include "planner.h"
#include "cache.h"

using std::vector;

class Page;

class Book : public QObject
{
    Q_OBJECT

public:
    Book(QObject *parent);
    ~Book();

    void open(const vector<FileInfo> &files);

    Page *getPageOne();
    Page *getPageTwo();

    Page *getPageToPrecache();

    void turnPageForward();
    void turnPageBackward();
    void turnPageForwardOnePage();
    void turnPageToStart();
    void turnPageToEnd();

    bool atStart();
    bool atEnd();

public slots:
    void resetDisplayArea(const QSize &area);

signals:
    void currentPagesReset();
    void currentPagesReplanned();

private:
    void finishTurningPage();

private:
    PageConductor _pageConductor;
    Planner _planner;
    Cache _cache;

    int _numPages;
    vector<Page*> _pages;
};

#endif
