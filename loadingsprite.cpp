#include "loadingsprite.h"

#include <QPainter>

LoadingSprite::LoadingSprite()
{
}


LoadingSprite::~LoadingSprite()
{
}

void LoadingSprite::setGeometry(const QRect &rect)
{
    _rect = rect;
}

void LoadingSprite::paint(QPainter *painter, const QRect &updateRect)
{
    painter->setBrush(Qt::magenta);
    painter->drawRect(_rect);
}
