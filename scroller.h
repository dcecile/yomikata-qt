#ifndef SCROLLER_H
#define SCROLLER_H

#include <QObject>

#include <QPointF>
#include <QTimer>
#include <QTime>

class QAbstractScrollArea;
class QScrollBar;
class QWidget;

class Scroller: public QObject
{
    Q_OBJECT

public:
    Scroller(QAbstractScrollArea *parent);
    ~Scroller();

    void stopScrolling();

private slots:
    void updateWidget();
    void timeStep();

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void moved(QPointF pos);

private:
    QAbstractScrollArea *_parent;
    QWidget *_viewport;
    QScrollBar *_hBar;
    QScrollBar *_vBar;
    QPointF _lastMousePos;

    QPointF _velocity;
    QPointF _scrollPos;
    QPointF _lastUpdatedScrollPos;

    QTimer _updateTimer;
    QTime _mouseTime;
    QTime _slideTime;
};

#endif
