#ifndef DEPICTER_H
#define DEPICTER_H

#include <QWidget>

#include "lens.h"

class Page;

class Depicter : public QWidget
{
    Q_OBJECT

public:
    Depicter(QWidget *parent);
    ~Depicter();

    void depict(Page *pageOne, Page *pageTwo);

public slots:
    void currentPagesReplanned();

signals:
    void displayAreaChanged(const QSize &newSize);
    void pageNeedsDecoding(Page *page);

private:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void pixmapUpdated(Page *page);
    void updateLensArea(const QRect &zone);

private:
    Lens _lensOne;
    Lens _lensTwo;

    QTime _resizeTime;
    bool _resizePainted;

    Page *_pageOne;
    Page *_pageTwo;
    bool _twoPages;
};

#endif
