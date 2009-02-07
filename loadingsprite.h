#ifndef LOADINGSPRITE_H
#define LOADINGSPRITE_H

#include <QTime>
#include <QPixmap>

class QPainter;

class LoadingSprite
{
public:
    LoadingSprite(QColor foreground, QColor background);
    ~LoadingSprite();

    void setGeometry(const QRect &rect);
    void paint(QPainter *painter, const QRect &updateRect);

private:
    static const int TILE_SIZE;
    static const double LINE_WIDTH;
    static const double SPEED;
    QTime _clock;
    QPixmap _tile;
    QRect _rect;
};

#endif
