#ifndef PAGESPRITE_H
#define PAGESPRITE_H

#include <QPixmap>

class PageSprite
{
public:
    PageSprite();
    ~PageSprite();

    void setPixmap(const QPixmap &pixmap);
    void setTopLeft(const QPoint &topLeft);
    void paint(QPainter *painter, const QRect &updateRect);

private:
    QPixmap _pixmap;
    QPoint _topLeft;
};

#endif
