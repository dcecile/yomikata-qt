#ifndef MAINWINDOWKDE_H
#define MAINWINDOWKDE_H

#include <KAction>
#include <KSelectAction>

#include "mainwindow.h"

class MainWindowKde : public MainWindow
{
    Q_OBJECT

public:
    MainWindowKde(const QString &initialArg, QWidget *parent = NULL);
    ~MainWindowKde();

private slots:
    void open();
    void toggleFullScreen(bool checked);
    void toggleMenubar(bool checked);

private:
    void createActions();
    void setAppDefaults();

private:
    QAction *_pageForwardAction;
    QAction *_pageBackwardAction;
    KAction *_pageLeftAction;
    KAction *_pageRightAction;

    KAction *_zoomToggleAction;
    KAction *_zoomInAction;
    KAction *_zoomOutAction;

    KSelectAction *_viewSelectAction;
    KAction *_singlePageAction;
    KAction *_comicsViewAction;
    KAction *_mangaViewAction;

};

#endif
