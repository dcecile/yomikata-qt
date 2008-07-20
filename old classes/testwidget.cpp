#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <KDebug>
#include <cmath>

#include "testwidget.h"

TestWidget::TestWidget(QWidget *parent)
    :QWidget(parent)
{
    init();

    setMouseTracking(true);
    setCursor(Qt::BlankCursor);

    restartTimer();

    _time.start();
}

void TestWidget::init()
{
    _offset.setX(width()/2);
    _offset.setY(height()/2);
    _velocity.setX(0.0);
    _velocity.setY(0.0);
    _newMousePath = true;
}

void TestWidget::restartTimer()
{
    const int STEP = 20;
    _timer.stop();
    _timer.start(STEP, this);
}

void TestWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    const int SIZE = 20;
    painter.drawEllipse(int(_offset.x()+0.5) - SIZE/2, int(_offset.y()+0.5) - SIZE/2, SIZE, SIZE);
    //kDebug()<<"Paint "<<_offset<<endl;
}

void TestWidget::enterEvent(QEvent *)
{
    _newMousePath = true;
}

void TestWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (_newMousePath) {
        _newMousePath = false;
        _previousMousePosition = event->pos();
        _time.restart();
        //kDebug()<<"Mouse started path "<<event->pos()<<endl;

    } else {
        qreal time = _time.elapsed();
        _time.restart();

        QPointF force = QPointF(event->pos() - _previousMousePosition);
        force /= 2000.0;

        QPointF delta = _velocity * time + 0.5 * force * time * time;
        _velocity *= exp(-time/500.0);
        if (fabs(_velocity.x()) < 2.0/1000.0) { _velocity.setX(0.0); }
        if (fabs(_velocity.y()) < 2.0/1000.0) { _velocity.setY(0.0); }
        _velocity += force * time;

        if (_offset.x() + delta.x() > width()) {
            delta.setX(qreal(width()) - _offset.x());
            _velocity.setX(0.0);
        } else if (_offset.x() + delta.x() < 0.0) {
            delta.setX(-_offset.x());
            _velocity.setX(0.0);
        }
        if (_offset.y() + delta.y() > height()) {
            delta.setY(qreal(height()) - _offset.y());
            _velocity.setY(0.0);
        } else if (_offset.y() + delta.y() < 0.0) {
            delta.setY(-_offset.y());
            _velocity.setY(0.0);
        }

        if (!delta.isNull()) {
            _offset += delta;
            update();
        }

        restartTimer();

        _previousMousePosition = event->pos();
    }
}

void TestWidget::resizeEvent(QResizeEvent *)
{
    init();
    update();
}

void TestWidget::timerEvent(QTimerEvent *)
{
    qreal time = _time.elapsed();
    _time.restart();

    QPointF delta = _velocity * time;
    _velocity *= exp(-time/500.0);
    if (fabs(_velocity.x()) < 2.0/1000.0) { _velocity.setX(0.0); }
    if (fabs(_velocity.y()) < 2.0/1000.0) { _velocity.setY(0.0); }

    if (_offset.x() + delta.x() > width()) {
        delta.setX(qreal(width()) - _offset.x());
        _velocity.setX(0.0);
    } else if (_offset.x() + delta.x() < 0.0) {
        delta.setX(-_offset.x());
        _velocity.setX(0.0);
    }
    if (_offset.y() + delta.y() > height()) {
        delta.setY(qreal(height()) - _offset.y());
        _velocity.setY(0.0);
    } else if (_offset.y() + delta.y() < 0.0) {
        delta.setY(-_offset.y());
        _velocity.setY(0.0);
    }

    if (!delta.isNull()) {
        _offset += delta;
        update();
    }
}

#include "testwidget.moc"
