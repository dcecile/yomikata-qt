#ifndef LOADINGSPRITE_H
#define LOADINGSPRITE_H

#include <QRect>

class QPainter;

class LoadingSprite
{
public:
    LoadingSprite();
    ~LoadingSprite();

    void setGeometry(const QRect &rect);
    void paint(QPainter *painter, const QRect &updateRect);

private:
    QRect _rect;
};

#endif
