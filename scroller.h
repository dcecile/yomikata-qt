#ifndef SCROLLER_H
#define SCROLLER_H

#include <QObject>

#include <QSizeF>
#include <QPointF>
#include <QTime>

class QWidget;

class Scroller: public QObject
{
    Q_OBJECT

public:
    Scroller(QWidget *parent);
    ~Scroller();

    void reset(const QSize &extent);

    QPoint position();

signals:
    void enableRefresh(bool enable);

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void moved(const QPointF &pos);
    void timeStep();
    void enforceBounds();

private:
    QWidget *_parent;
    QPointF _lastMousePos;

    QSizeF _extent;
    QPointF _velocity;
    QPointF _scrollPos;

    QTime _mouseTime;
    QTime _slideTime;
};

#endif
