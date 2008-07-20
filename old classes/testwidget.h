#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QBasicTimer>
#include <QTime>

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    TestWidget(QWidget *parent = 0);

private:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void resizeEvent(QResizeEvent *event);

    void timerEvent(QTimerEvent *event);

    void init();
    void restartTimer();

private:
    QPointF _offset;
    QPointF _velocity;
    bool _newMousePath;
    QPoint _previousMousePosition;

    QBasicTimer _timer;
    QTime _time;
};

#endif
