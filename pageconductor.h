#ifndef PAGECONDUCTOR_H
#define PAGECONDUCTOR_H

#include <QObject>
#include <QMutex>

#include <vector>

using std::vector;

class PageConductor : public QObject
{
    Q_OBJECT

public:
    PageConductor(QObject *parent);
    ~PageConductor();

    void initialize(int numPages);

    int getPageOne();
    int getPageTwo();

    int getPairedPage(int originalPage);

    void setPageDouble(int pageNumber);
    void setPageNotDouble(int pageNumber);

    void turnPageForward();
    void turnPageBackward();
    void turnPageForwardOnePage();
    void turnPageToStart();
    void turnPageToEnd();

    bool atStart();
    bool atEnd();

signals:
    void pagePairsChanged(int lowerPageNumber, int upperPageNumber);

private:
    void finishTurningPage();

private:
    struct PageInfo {
        int prevDouble;
        int nextDouble;
    };
private:
    vector<PageInfo> _pages;

    QMutex _mutex;

    bool _twoPageMode;

    int _numPages;
    int _primaryPage;
    int _targetPage[2];
};

#endif
