#include "loadingwidget.h"

#include <QPainter>

#include <cmath>

const int LoadingWidget::TILE_SIZE = 64;
const double LoadingWidget::LINE_WIDTH = 0.2;
const double LoadingWidget::SPEED = 0.03;

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent), _tile(TILE_SIZE, TILE_SIZE), _fps(16, 100)
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
        points[i] *= double(TILE_SIZE);
    }

    _tile.fill(palette().color(QPalette::Base));

    QPainter painter(&_tile);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor lineColor = palette().color(QPalette::Highlight);
    lineColor.setAlphaF(0.5);
    painter.setPen(QPen(lineColor, double(TILE_SIZE) * LINE_WIDTH));

    painter.drawLines(points);

    _clock.start();
    _frameClock.start();
}


LoadingWidget::~LoadingWidget()
{
}

void LoadingWidget::paintEvent(QPaintEvent *event)
{
    double offset = fmod(_clock.elapsed() * SPEED, TILE_SIZE);

    QPainter painter(this);

    painter.translate(QPoint(0, -int(offset + 0.5)));

    /*
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(_tile));

    painter.drawRect(rect().adjusted(0, 0, 0, TILE_SIZE));
    //*/

    //*
    int cols = (width() + TILE_SIZE - 1) / TILE_SIZE + 1;
    int rows = (height() + TILE_SIZE - 1) / TILE_SIZE + 1;

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            painter.drawPixmap(x * TILE_SIZE, y * TILE_SIZE, _tile);
        }
    }
    //*/

    update();

    _fps.addSample(_frameClock.elapsed());
    _frameClock.restart();
}

#include "loadingwidget.moc"
