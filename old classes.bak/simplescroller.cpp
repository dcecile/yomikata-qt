#include <KDebug>

#include "simplescroller.h"

SimpleScroller::SimpleScroller(QObject *parent)
    :DisplayScroller(parent)
{
}

SimpleScroller::~SimpleScroller()
{
}

void SimpleScroller::mouseMoved(const QPoint &distance)
{
    QPoint delta(distance);
    delta *= zoom();

    if (_offset.x() + delta.x() < bounds().left()) {
        delta.setX(bounds().left() - _offset.x());
    } else if (_offset.x() + delta.x() > bounds().right()) {
        delta.setX(bounds().right() - _offset.x());
    }
    if (_offset.y() + delta.y() > bounds().bottom()) {
        delta.setY(bounds().bottom() - _offset.y());
    } else if (_offset.y() + delta.y() < bounds().top()) {
        delta.setY(bounds().top() - _offset.y());
    }

    if (!delta.isNull()) {
        _offset += delta;
        Q_ASSERT(bounds().contains(_offset));
        emit moved(_offset);
    }
}

void SimpleScroller::resetPosition(const QPoint &startPoint)
{
    _offset = startPoint;
}

#include "simplescroller.moc"
