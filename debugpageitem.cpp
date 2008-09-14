#include "debugpageitem.h"

#include <QBrush>
#include <QFont>

DebugPageItem::DebugPageItem(qreal x, qreal y, int page)
    : QGraphicsRectItem(QRect(0, 0, 50, 70))
{
    // Draggable
    setFlag(QGraphicsItem::ItemIsMovable);

    // Set position
    setPos(x, y);

    // Set the colour
    setBrush(QBrush(Qt::blue));

    // Create the text
    _text = new QGraphicsTextItem(QString("%1").arg(page), this);
    QFont font = _text->font();
    font.setBold(true);
    _text->setFont(font);
    _text->setDefaultTextColor(QColor(Qt::white));
    _text->setPos(25 - _text->boundingRect().width()/2.0, 45 - _text->boundingRect().height());
}

DebugPageItem::~DebugPageItem()
{
}
