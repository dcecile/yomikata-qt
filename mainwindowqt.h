#ifndef MAINWINDOWQT_H
#define MAINWINDOWQT_H

#include "mainwindow.h"

class MainWindowQt : public MainWindow
{
    Q_OBJECT

public:
    MainWindowQt(const QString &initialArg, QWidget *parent = NULL);
    ~MainWindowQt();

private slots:
    void open();
};

#endif
