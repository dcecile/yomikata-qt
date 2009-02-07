#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QTime>
#include <QPixmap>

#include "mean.h"

class LoadingWidget : public QWidget
{
    Q_OBJECT

public:
    LoadingWidget(QWidget *parent = NULL);
    ~LoadingWidget();

private:
    void paintEvent(QPaintEvent *event);

private:
    static const int TILE_SIZE;
    static const double LINE_WIDTH;
    static const double SPEED;
    QTime _clock;
    QPixmap _tile;
    Mean _fps;
    QTime _frameClock;
};

#endif
