#include <KDebug>

#include "displayscroller.h"

DisplayScroller::DisplayScroller(QObject *parent)
    :QObject(parent)
{
}

DisplayScroller::~DisplayScroller()
{
}

void DisplayScroller::reset(const QRect &bounds, qreal zoom, const QPoint &startPoint)
{
    kDebug()<<"Reset "<<bounds<<startPoint<<zoom<<endl;
    _bounds = bounds;
    _zoom = zoom;
    resetPosition(startPoint);
}

#include "displayscroller.moc"
