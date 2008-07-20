#ifndef YOMIKATA_H
#define YOMIKATA_H

#include <KMainWindow>
#include <KTextEdit>
#include <QLabel>
#include <QBoxLayout>

#include "pageloader.h"

class Yomikata : public KMainWindow
{
    Q_OBJECT

public:
    Yomikata(QWidget *parent=0);

private slots:
    void open();

    void pagesListed(int initialPosition, int numPages);
    void pageRead(int pageNum);
    void pageReadFailed(int pageNum);

private:
    void resizeEvent(QResizeEvent *event);

    void setupActions();

private:
    PageLoader _pageLoader;

    QWidget *_innerWidget;
    QWidget *_spacerA;
    QWidget *_spacerB;
    QLabel *_imageLabel;
    QBoxLayout *_pageLayout;
};

#endif
