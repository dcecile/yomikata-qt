#include "loadingsprite.h"

#include <QPainter>

const int LoadingSprite::TILE_SIZE = 64;
const double LoadingSprite::LINE_WIDTH = 0.2;
const double LoadingSprite::SPEED = 0.03;

LoadingSprite::LoadingSprite(QColor foreground, QColor background)
    : _tile(TILE_SIZE, TILE_SIZE)
{
    QVector<QPointF> points;

    /*
    points.push_back(QPointF(0.5, 0.0));
    points.push_back(QPointF(0.0, 0.25));

    points.push_back(QPointF(1.0, 0.25));
    points.push_back(QPointF(0.0, 0.75));

    points.push_back(QPointF(1.0, 0.75));
    points.push_back(QPointF(0.5, 1.0));
    //*/

    //*
    points.push_back(QPointF(0.5, 0.0));
    points.push_back(QPointF(0.0, 0.5));
    points.push_back(QPointF(1.0, 0.5));
    points.push_back(QPointF(0.5, 1.0));
    //*/

    /*
    points.push_back(QPointF(0.25, 0.0));
    points.push_back(QPointF(0.0, 0.25));
    points.push_back(QPointF(1.0, 0.25));
    points.push_back(QPointF(0.25, 1.0));

    points.push_back(QPointF(0.75, 0.0));
    points.push_back(QPointF(0.0, 0.75));
    points.push_back(QPointF(1.0, 0.75));
    points.push_back(QPointF(0.75, 1.0));


    points.push_back(QPointF(0.5, -0.5));
    points.push_back(QPointF(-0.5, 0.5));
    points.push_back(QPointF(1.5, 0.5));
    points.push_back(QPointF(0.5, 1.5));

    points.push_back(QPointF(1.0, 0.0));
    points.push_back(QPointF(0.0, 1.0));
    //*/

    for (int i = 0; i < points.size(); i++)
    {
        points[i].setX(1.0 - points[i].x());
        points[i] *= double(TILE_SIZE);
    }

    _tile.fill(background);

    QPainter painter(&_tile);
    painter.setRenderHint(QPainter::Antialiasing);

    foreground.setAlphaF(0.2);
    painter.setPen(QPen(foreground, double(TILE_SIZE) * LINE_WIDTH));

    painter.drawLines(points);

    _clock.start();
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
    QRect paintRect = _rect & updateRect;

    int minx = (paintRect.left() + TILE_SIZE - 1) / TILE_SIZE - 1;
    int miny = (paintRect.top() + TILE_SIZE - 1) / TILE_SIZE - 2;
    int maxx = (paintRect.right() + TILE_SIZE - 1) / TILE_SIZE;
    int maxy = (paintRect.bottom() + TILE_SIZE - 1) / TILE_SIZE + 1;

    int offset = int(double(_clock.elapsed()) * SPEED + 0.5) % TILE_SIZE;

    QRect fullSource(0, 0, TILE_SIZE, TILE_SIZE);
    QRect source;
    QPoint topLeft;

    for (int y = miny; y < maxy; y++)
    {
        for (int x = minx; x < maxx; x++)
        {
            topLeft = QPoint(x * TILE_SIZE, y * TILE_SIZE + offset);
            source = fullSource & paintRect.translated(-topLeft);

            if (!source.isEmpty())
            {
                topLeft += source.topLeft();
                painter->drawPixmap(topLeft, _tile, source);
            }
        }
    }
}
