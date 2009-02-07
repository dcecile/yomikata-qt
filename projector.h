#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QWidget>

#include <QTimer>

#include "scroller.h"
#include "loadingsprite.h"
#include "pagesprite.h"

class Projector : public QWidget
{
    Q_OBJECT

public:
    Projector(QWidget *parent = NULL);
    ~Projector();

    void showBlank();

    void showLoading0(const QRect &rect);
    void showLoading1(const QRect &rect);
    void showPage0(const QRect &rect, QPixmap pixmap);
    void showPage1(const QRect &rect, QPixmap pixmap);

    void updatePosition0(const QRect &rect);
    void updatePosition1(const QRect &rect);

    QSize sizeHint() const;
    int heightForWidth(int width) const;

public slots:
    void pagesChanged();

signals:
    void resized(const QSize &size);

private:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);

private slots:
    void refresh();

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
};

#endif
