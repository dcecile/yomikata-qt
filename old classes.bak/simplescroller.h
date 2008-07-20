#ifndef SIMPLESCROLLER_H
#define SIMPLESCROLLER_H

#include "displayscroller.h"

class SimpleScroller : public DisplayScroller
{
    Q_OBJECT

public:
    SimpleScroller(QObject *parent = 0);
    ~SimpleScroller();

    void mouseMoved(const QPoint &distance);

    void stop() {}

private:
    void resetPosition(const QPoint &startPoint);

private:
    QPoint _offset;
};

#endif
