#include "debugpageitem.h"

#include <QBrush>
#include <QFont>

DebugPageItem::DebugPageItem(qreal x, qreal y, bool active, bool dual, int page)
    : QGraphicsRectItem(QRect(0, 0, (dual ?100 :50), 70))
{
    // Draggable
    setFlag(QGraphicsItem::ItemIsMovable);

    // Set position
    setPos(x, y);

    // Set the colour
    if (active)
    {
        setBrush(QBrush(QColor(0x8200D2)));
    }
    else
    {
        setBrush(QBrush(QColor(0x294695)));
    }

    // Create the text
    _text = new QGraphicsTextItem(QString("%1").arg(page), this);
    QFont font = _text->font();
    font.setBold(true);
    _text->setFont(font);
    _text->setDefaultTextColor(QColor(Qt::white));
    _text->setPos((dual ?50 : 25) - _text->boundingRect().width()/2.0, 45 - _text->boundingRect().height());
}

DebugPageItem::~DebugPageItem()
{
}
