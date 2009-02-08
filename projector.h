#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QWidget>

#include <QTimer>

#include "scroller.h"
#include "loadingsprite.h"
#include "pagesprite.h"

struct DisplayMetrics;

class Projector : public QWidget
{
    Q_OBJECT

public:
    Projector(QWidget *parent = NULL);
    ~Projector();

    void setDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1);
    void updateDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1);

    QSize sizeHint() const;
    int heightForWidth(int width) const;

signals:
    void resized(const QSize &fullSize, const QSize &viewSize);

private:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void refresh();
    void enableRefresh(bool enable);

private:
    static const double MAGNIFICATION;
    static const double FRAMES_PER_SECOND;
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
