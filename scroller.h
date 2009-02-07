#ifndef SCROLLER_H
#define SCROLLER_H

#include <QObject>

#include <QPointF>
#include <QTime>
#include <QSize>

class QWidget;

class Scroller: public QObject
{
    Q_OBJECT

public:
    Scroller(QWidget *parent);
    ~Scroller();

    void setExtent(const QSize &size);
    void reset();
    void stopScrolling();

    QPoint position();

signals:
    void enableUpdates(bool enable);

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void moved(const QPointF &pos);
    void timeStep();

private:
    QWidget *_parent;
    QPointF _lastMousePos;

    QPointF _velocity;
    QPointF _scrollPos;
    QPointF _lastUpdatedScrollPos;

    QTime _mouseTime;
    QTime _slideTime;
};

#endif
