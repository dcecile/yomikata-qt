#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <vector>

#include "debugpageitem.h"
#include "book.h"

using std::vector;

class DebugWidget : public QGraphicsView
{
    Q_OBJECT

    public:
        DebugWidget(Book &_book, QWidget *parent = NULL);
        ~DebugWidget();

        QSize sizeHint() const;

    private:
        void resizeEvent(QResizeEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void showEvent(QShowEvent *event);
        void hideEvent(QHideEvent *event);

    private slots:
        void setup();

    private:
        Book &_book;

        QGraphicsScene _scene;
        vector<DebugPageItem *> _pages;
};

#endif
