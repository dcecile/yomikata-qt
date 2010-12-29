#include "scroller.h"

#include <QWidget>
#include <QMouseEvent>

#include <algorithm>

#include <cmath>

#include "debug.h"

using std::min;
using std::max;

Scroller::Scroller(QObject *parent)
    : QObject(parent)
{
    // Start stopped
    _extent = QSizeF(0.0, 0.0);
    _velocity = QPointF(0.0, 0.0);
    _scrollPos = QPointF(0.0, 0.0);
    _isReset = true;

    // Start the clocks
    _slideTime.start();
    _mouseTime.start();
}

Scroller::~Scroller()
{
}

void Scroller::reset(const QSize &extent)
{
    // New bounds
    _extent = extent;

    // Reset to start of pages
    _scrollPos.setX(_extent.width());
    _scrollPos.setY(0.0);
}

void Scroller::update(const QSize &extent)
{
    // If extent changed, go back to the top right
    // ie. a new total width has been calculated
    if (_extent != extent)
    {
        reset(extent);
    }
}

QPoint Scroller::position()
{
    // Finish with the last force parameters
    timeStep();

    // Disable refresh if stopped
    if (_velocity.isNull())
    {
        emit enableRefresh(false);
    }

    // Return the current position
    return _scrollPos.toPoint();
}

void Scroller::mouseMoved(const QPointF &pos)
{
    // Finish with the last force parameters
    timeStep();

    // Find the time spent moving
    double time = _mouseTime.elapsed();
    _mouseTime.restart();
    //debug()<<"Mouse move: "<<time<<" ms";

    // Cap out the time
    time = min(time, 20.0);

    // Find the distance
    QPointF distance = _isReset ? QPointF(0.0, 0.0) : _lastMousePos - pos;
    _lastMousePos = pos;
    _isReset = false;

    // Calculate the force
    const double FORCE_FACTOR = -0.02;
    QPointF force = distance * FORCE_FACTOR;

    _velocity += force * time;
    _scrollPos += 0.5 * force * time * time;

    // Stay within the extent
    enforceBounds();

    // Enable refresh
    emit enableRefresh(!_velocity.isNull());
}

void Scroller::resetMouse()
{
    _isReset = true;
    _velocity = QPointF(0.0, 0.0);
    emit enableRefresh(false);
}

void Scroller::timeStep()
{
    // Get the elapsed time
    double totalTime = _slideTime.elapsed();
    _slideTime.restart();

    // Cap it out
    totalTime = min(totalTime, 1000.0);

    // Evaluate each time step
    const double TIME_STEP = 0.01;
    const double DRAG = 0.17;
    const double CUTOFF = 2.0/1000.0;
    QPointF totalForce;

    double time = min(totalTime, TIME_STEP);

    while (totalTime > 0.0)
    {
        // Add in friction
        totalForce = - _velocity * DRAG;

        // Simple acceleration
        _velocity += totalForce * time;
        _scrollPos += _velocity * time + 0.5 * totalForce * time * time;

        // Cut off small values
        if (fabs(_velocity.x()) < CUTOFF)
        {
            _velocity.setX(0.0);
        }

        if (fabs(_velocity.y()) < CUTOFF)
        {
            _velocity.setY(0.0);
        }

        // Go to the next time step
        totalTime -= time;
        time = min(totalTime, TIME_STEP);
    }

    // Stay within the extent
    enforceBounds();
}

void Scroller::enforceBounds()
{
    if (_scrollPos.x() < 0.0)
    {
        _scrollPos.setX(0.0);
        _velocity.setX(0.0);
    }
    else if (_scrollPos.x() > _extent.width())
    {
        _scrollPos.setX(_extent.width());
        _velocity.setX(0.0);
    }

    if (_scrollPos.y() < 0.0)
    {
        _scrollPos.setY(0.0);
        _velocity.setY(0.0);
    }
    else if (_scrollPos.y() > _extent.height())
    {
        _scrollPos.setY(_extent.height());
        _velocity.setY(0.0);
    }
}

#include "scroller.moc"
