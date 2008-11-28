#include "scroller.h"

#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QApplication>

#include <algorithm>

#include <cmath>

#include "debug.h"

using std::min;
using std::max;

Scroller::Scroller(QAbstractScrollArea *parent)
    : QObject(parent)
{
    _parent = parent;
    _viewport = _parent->viewport();
    _hBar = _parent->horizontalScrollBar();
    _vBar = _parent->verticalScrollBar();
    debug()<<"Bars"<<_hBar<<_vBar;

    // Subscribe to mouse movement
    _viewport->setMouseTracking(true);

    // Hide the cursor
    _viewport->setCursor(Qt::BlankCursor);

    // Become an event filter
    _viewport->installEventFilter(this);

    // Start stopped
    _velocity = QPointF(0.0, 0.0);
    _scrollPos = QPointF(0.0, 0.0);
    _lastUpdatedScrollPos = QPointF(0.0, 0.0);

    // Start the timers
    _slideTime.start();
    _mouseTime.start();

    // Set up the timer
    _updateTimer.setInterval(int(1000.0 / 30.0 + 0.5));
    _updateTimer.setSingleShot(true);
    connect(&_updateTimer, SIGNAL(timeout()), SLOT(updateWidget()));
    _updateTimer.start();
}

Scroller::~Scroller()
{
}

void Scroller::stopScrolling()
{
    _velocity = QPointF(0.0, 0.0);
}

bool Scroller::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == _viewport)
    {
        if (event->type() == QEvent::MouseMove)
        {
            // Found a mouse event of the parent
            QMouseEvent *mouseEvent = (QMouseEvent *) event;
            moved(mouseEvent->posF());
        }
    }
    return false;
}

void Scroller::moved(QPointF pos)
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
    QPointF distance = _lastMousePos - pos;
    _lastMousePos = pos;

    // Calculate the force
    //const double FORCE_FACTOR = -0.01;
    const double FORCE_FACTOR = -0.02;
    QPointF force = distance * FORCE_FACTOR;

    _velocity += force * time;
    _scrollPos += 0.5 * force * time * time;
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
    //const double FRICTION = 0.1;
    const double FRICTION = 0.2;
    //const double FORCE_DECAY = 4.0;
    const double CUTOFF = 2.0/1000.0;
    QPointF totalForce;

    double time = min(totalTime, TIME_STEP);

    while (totalTime > 0.0)
    {
        // Add in friction
        totalForce = - _velocity * FRICTION;
        //_velocity *= exp(-time/400.0);

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
}

void Scroller::updateWidget()
{
    // Check for stopped
    if (_velocity.isNull())
    {
        _slideTime.restart();
        _updateTimer.start();
        return;
    }

    // Pump the events, to make sure input gets handled
    QApplication::syncX();
    QApplication::processEvents();

    // Do the timestep
    timeStep();

    // Update the scroll positions
    QPoint delta = (_scrollPos - _lastUpdatedScrollPos).toPoint();
    _hBar->setValue(_hBar->value() + delta.x());
    _vBar->setValue(_vBar->value() + delta.y());
    _lastUpdatedScrollPos = _scrollPos;

    // Start the update timer again
    _updateTimer.start();
}

#include "scroller.moc"
