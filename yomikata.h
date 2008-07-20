#ifndef YOMIKATA_H
#define YOMIKATA_H

#include <KMainWindow>
#include <KTextEdit>
#include <QLabel>
#include <QBoxLayout>
#include <KAction>
#include <QBasicTimer>

#include "pageloader.h"
#include "pagedisplay.h"

class Yomikata : public KMainWindow
{
    Q_OBJECT

public:
    Yomikata(QWidget *parent=0);

private slots:
    void open();
    void toggleFullScreen(bool checked);
    void toggleMenubar(bool checked);

    void pagesListed(int initialPosition, int numPages);
    void pageRead(int pageNum);
    void pageReadFailed(int pageNum);

    void pageLeft();
    void pageRight();

private:
    void changePage();

    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);

    void timerEvent(QTimerEvent *event);

    void synchronizeActions();

    void createActions();

    void setAppDefaults();

private:
    enum PageMode {SingleMode, ComicsMode, MangaMode};
    PageMode _pageMode;

    int _numPages;

    int _targetPage;
    int _targetPageB;

    PageLoader _pageLoader;

    PageDisplay *_pageDisplay;

    KAction *_pageLeftAction;
    KAction *_pageRightAction;

    QBasicTimer _pageNumberTimer;
    QLabel *_pageNumberLabel;
};

#endif
