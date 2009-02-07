#include "pagesprite.h"

#include <QPainter>

PageSprite::PageSprite()
{
}


PageSprite::~PageSprite()
{
}

void PageSprite::setPixmap(const QPixmap &pixmap)
{
    _pixmap = pixmap;
}

void PageSprite::setTopLeft(const QPoint &topLeft)
{
    _topLeft = topLeft;
}

void PageSprite::paint(QPainter *painter, const QRect &updateRect)
{
    painter->drawPixmap(_topLeft, _pixmap);
}
