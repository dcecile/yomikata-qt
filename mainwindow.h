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

class Steward;

class MainWindow: public MainWindowAncestor
{
    Q_OBJECT

public:
    MainWindow(const QString &initialArg, QWidget *parent = NULL);
    ~MainWindow();

    QSize sizeHint() const;

signals:
    void nextPage();
    void previousPage();
    void shiftNextPage();

protected:
    void setSource(const QString &filename);

protected slots:
    virtual void open() = 0;
    virtual void fullscreen(bool toggled) = 0;

private:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Steward *_steward;
    bool _zoomToggleEnabled;
    bool _zoomInEnabled;
    bool _zoomOutEnabled;
};

#endif
