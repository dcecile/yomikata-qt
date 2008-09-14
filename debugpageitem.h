#ifndef DEBUGPAGEITEM_H
#define DEBUGPAGEITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class DebugPageItem: public QGraphicsRectItem
{
    public:
        DebugPageItem(qreal x, qreal y, int page);
        ~DebugPageItem();

    private:
        QGraphicsTextItem *_text;
};

#endif
