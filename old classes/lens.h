#ifndef LENS_H
#define LENS_H

#include <QObject>
#include <QPainter>
#include <QTimeLine>
#include <QTime>

#include "stats.h"

/**
 * @ingroup mod_presenter
 * @brief Displays a page
 */
class Lens : public QObject
{
    Q_OBJECT

public:
    Lens(QObject *parent);

    void close();
    void wait(int pageNum, const QRect &displayRect);
    void focus(QPixmap pixmap, QPixmap thumbnail, const QRect &displayRect);

    void reposition(const QRect &displayRect);

    void paint(QPainter *painter, const QRect &paintRect);

    ~Lens();

signals:
    void update(const QRect &zone);

private:
    void paintWaiting(QPainter *painter);
    void paintFocused(QPainter *painter);

private slots:
    void loadingAnimation(qreal value);

private:
    static const int ANIM_PERIOD = 8000;
    static const QSize THUMBNAIL_SIZE;

private:
    enum State { Closed, Waiting, Focused } _state;

    int _pageNum;
    QTimeLine _loadingTimeLine;
    qreal _animationValue;

    QRect _displayRect;

    QPixmap _pixmap;
    QPixmap _thumbnail;

    QTime _resizeTime;
};

#endif
