#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QObject>

#include <QTimer>

#include "scroller.h"
#include "loadingsprite.h"
#include "pagesprite.h"

struct DisplayMetrics;

class Projector : public QObject
{
    Q_OBJECT

public:
    Projector(QObject *parent = NULL);
    ~Projector();

    void setDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1);
    void updateDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1);

    void retrieveDisplay(QRect *rect0, QRect *rect1);

    void setViewSize(const QSize &size);
    QSize viewSize() const;
    QSize fullSize() const;

    void paint(QPainter *painter, const QRect &updateRect);

    void mouseMoved(const QPointF &pos);

signals:
    void update();
    void repaint();

private slots:
    void refresh();
    void enableRefresh(bool enable);

private:
    static const double MAGNIFICATION;
    static const double FRAMES_PER_SECOND;
    QSize _viewSize;
    QSize _fullSize;
    bool _isShown[2];
    bool _isLoading[2];
    QRect _placement[2];
    Scroller _scroller;
    LoadingSprite _loadingSprite;
    PageSprite _pageSprite0;
    PageSprite _pageSprite1;
    QTimer _refreshTimer;
    bool _refreshRequested;
};

#endif
