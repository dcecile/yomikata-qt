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
    Yomikata(const QString &initialArg, QWidget *parent=0);

private slots:
    void open();
    void toggleFullScreen(bool checked);
    void toggleMenubar(bool checked);

    void pageReadFailed(int pageNum);

    void enableForward(bool enabled);
    void enableBackward(bool enabled);

private:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

    void createActions();

    void setAppDefaults();

private:
    enum PageMode {SingleMode, ComicsMode, MangaMode};
    PageMode _pageMode;

    PageLoader _pageLoader;

    PageDisplay *_pageDisplay;

    QAction *_pageForwardAction;
    QAction *_pageBackwardAction;
    KAction *_pageLeftAction;
    KAction *_pageRightAction;
    KAction *_pageToStartAction;
    KAction *_pageToEndAction;
};

#endif
