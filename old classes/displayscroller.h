#ifndef DISPLAYSCROLLER_H
#define DISPLAYSCROLLER_H

#include <QObject>
#include <QRect>

class DisplayScroller : public QObject
{
    Q_OBJECT

public:
    DisplayScroller(QObject *parent = 0);
    virtual ~DisplayScroller();

    void reset(const QRect &bounds, qreal zoom, const QPoint &startPoint);

    virtual void mouseMoved(const QPoint &distance) = 0;

    virtual void stop() = 0;

signals:
    void moved(const QPoint &newPosition);

protected:
    inline const QRect &bounds() const;
    inline qreal zoom() const;

    virtual void resetPosition(const QPoint &startPoint) = 0;

private:
    QRect _bounds;
    qreal _zoom;
};

const QRect &DisplayScroller::bounds() const
{
    return _bounds;
}
qreal DisplayScroller::zoom() const
{
    return _zoom;
}

#endif
