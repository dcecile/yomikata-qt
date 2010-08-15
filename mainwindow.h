#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "book.h"

class Steward;

class MainWindow: public QMainWindow
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
    void open();
    void fullscreen(bool toggled);

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
