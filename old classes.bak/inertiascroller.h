#ifndef INERTIASCROLLER_H
#define INERTIASCROLLER_H

#include <QPointF>
//#include <QBasicTimer>
#include <QTimer>
#include <QTime>

#include "displayscroller.h"

class InertiaScroller : public DisplayScroller
{
    Q_OBJECT

public:
    InertiaScroller(QObject *parent = 0);
    ~InertiaScroller();

    void mouseMoved(const QPoint &distance);

    void stop();

private:
    void resetPosition(const QPoint &startPoint);

    //void timerEvent(QTimerEvent *event);

    void restartTimer();

private slots:
    void timeout();
    void waitDone();

private:
    QPointF _offset;
    QPointF _velocity;

    //QBasicTimer _timer;
    QTimer _timer;
    QTimer _waitTimer;
    QTime _time;
};

#endif
