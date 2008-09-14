#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef QT_ONLY
    #include <QMainWindow>
    typedef QMainWindow MainWindowAncestor;
#else
    #include <KXmlGuiWindow>
    typedef KXmlGuiWindow MainWindowAncestor;
#endif

#include "book.h"

class MainWindow: public MainWindowAncestor
{
    Q_OBJECT

public:
    MainWindow(const QString &initialArg, QWidget *parent = NULL);
    ~MainWindow();

    QSize sizeHint() const;

protected:
    void setSource(const QString &initialFile);

private:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Book _book;

    bool _pageForwardEnabled;
    bool _pageBackwardEnabled;
    bool _zoomToggleEnabled;
    bool _zoomInEnabled;
    bool _zoomOutEnabled;
};

#endif
