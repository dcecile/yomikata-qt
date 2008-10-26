#include <KDebug>
#include <QCoreApplication>
#include <cmath>

#include "inertiascroller.h"

InertiaScroller::InertiaScroller(QObject *parent)
    :DisplayScroller(parent)
{
    _timer.setSingleShot(true);
    _waitTimer.setSingleShot(true);
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(&_waitTimer, SIGNAL(timeout()), this, SLOT(waitDone()));
}

InertiaScroller::~InertiaScroller()
{
}

void InertiaScroller::mouseMoved(const QPoint &distance)
{
    qreal time = _time.elapsed();
    _time.restart();
    kDebug()<<"Mouse move: "<<time<<" ms"<<endl;

    if (time > 30.0) { time = 30.0; }

    QPointF force = QPointF(distance);
    force *= zoom() / 4000.0;

    QPointF delta = _velocity * time + 0.5 * force * time * time;
    _velocity *= exp(-time/400.0);
    if (fabs(_velocity.x()) < 2.0/1000.0) { _velocity.setX(0.0); }
    if (fabs(_velocity.y()) < 2.0/1000.0) { _velocity.setY(0.0); }
    _velocity += force * time;

    if (_offset.x() + delta.x() < bounds().left()) {
        delta.setX(bounds().left() - _offset.x());
        _velocity.setX(0.0);
    } else if (_offset.x() + delta.x() > bounds().right()) {
        delta.setX(bounds().right() - _offset.x());
        _velocity.setX(0.0);
    }
    if (_offset.y() + delta.y() > bounds().bottom()) {
        delta.setY(bounds().bottom() - _offset.y());
        _velocity.setY(0.0);
    } else if (_offset.y() + delta.y() < bounds().top()) {
        delta.setY(bounds().top() - _offset.y());
        _velocity.setY(0.0);
    }

    if (!delta.isNull()) {
        _offset += delta;
        Q_ASSERT(bounds().contains(_offset.toPoint()));
        emit moved(_offset.toPoint());
    }

    restartTimer();
}

//void InertiaScroller::timerEvent(QTimerEvent *)
void InertiaScroller::timeout()
{
    // HACK: Double check that there's no pending evnets
    if (QCoreApplication::hasPendingEvents()) {
        kDebug()<<"Timer aborting"<<endl;
        // Wait before triggering again
        if (!_velocity.isNull()) {
            const int STEP = 40;
            _waitTimer.start(STEP);
        }
        return;
    }

    qreal time = _time.elapsed();
    _time.restart();
    kDebug()<<"Timer: "<<time<<" ms"<<endl;

    if (time > 60.0) { time = 60.0; }

    QPointF delta = _velocity * time;
    _velocity *= exp(-time/400.0);
    if (fabs(_velocity.x()) < 2.0/1000.0) { _velocity.setX(0.0); }
    if (fabs(_velocity.y()) < 2.0/1000.0) { _velocity.setY(0.0); }

    if (_offset.x() + delta.x() < bounds().left()) {
        delta.setX(bounds().left() - _offset.x());
        _velocity.setX(0.0);
    } else if (_offset.x() + delta.x() > bounds().right()) {
        delta.setX(bounds().right() - _offset.x());
        _velocity.setX(0.0);
    }
    if (_offset.y() + delta.y() > bounds().bottom()) {
        delta.setY(bounds().bottom() - _offset.y());
        _velocity.setY(0.0);
    } else if (_offset.y() + delta.y() < bounds().top()) {
        delta.setY(bounds().top() - _offset.y());
        _velocity.setY(0.0);
    }

    if (!delta.isNull()) {
        _offset += delta;
        Q_ASSERT(bounds().contains(_offset.toPoint()));
        emit moved(_offset.toPoint());
    }

    // Wait before triggering again
    if (!_velocity.isNull()) {
        const int STEP = 40;
        _waitTimer.start(STEP);
    }

    kDebug()<<"Done timer"<<endl;
    //restartTimer();
}

void InertiaScroller::waitDone()
{
    //kDebug()<<"Waitng done"<<endl;

    // Double check that there's no pending evnets
    if (QCoreApplication::hasPendingEvents()) {
        kDebug()<<"Waiting again"<<endl;
        // Wait before triggering again
        if (!_velocity.isNull()) {
            const int STEP = 40;
            _waitTimer.start(STEP);
        }
        return;
    }
    if (!_velocity.isNull()) {
        _timer.start(0);
    }
}

void InertiaScroller::resetPosition(const QPoint &startPoint)
{
    _offset = startPoint;
    _velocity = QPointF(0.0, 0.0);

    restartTimer();

    _time.restart();
}

void InertiaScroller::stop()
{
    _waitTimer.stop();
    _timer.stop();
}

void InertiaScroller::restartTimer()
{
    _waitTimer.stop();
    if (_velocity.isNull()) {
        //kDebug()<<"Stopping timer"<<endl;
        _timer.stop();
    } else {
        //kDebug()<<"Starting timer"<<endl;
        _timer.start(0);
    }
}

#include "inertiascroller.moc"
