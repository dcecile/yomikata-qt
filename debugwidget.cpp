#include "debugwidget.h"

#include "debug.h"

DebugWidget::DebugWidget(const Book &book, QWidget *parent)
    : QGraphicsView(parent), _book(book)
{
    // Create the scene
    _scene.setItemIndexMethod(QGraphicsScene::NoIndex);

    // Create pages
    int pos = 0;

    for (int i = 0; i < _book.numPages(); i++)
    {
        pos -= 60 + 5 * _book.pairedPageOffset(i);
        DebugPageItem *temp = new DebugPageItem(pos, 10, i);
        _pages.push_back(temp);
        _scene.addItem(temp);
    }

    // Bound the pages
    DebugPageItem *front = _pages.front();
    DebugPageItem *back = _pages.back();
    QPolygonF boundingPoly = front->mapToScene(front->boundingRect());
    boundingPoly = boundingPoly.united(back->mapToScene(back->boundingRect()));
    QRectF boundingRect = boundingPoly.boundingRect();
    boundingRect.adjust(-10, -10, 10, 10);
    _scene.setSceneRect(boundingRect);

    // Create the view
    setScene(&_scene);
    setRenderHint(QPainter::Antialiasing);
    setCacheMode(QGraphicsView::CacheBackground);
    setDragMode(QGraphicsView::NoDrag);
    setInteractive(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

DebugWidget::~DebugWidget()
{
}

QSize DebugWidget::sizeHint() const
{
    return QSize(400, 600);
}

void DebugWidget::resizeEvent(QResizeEvent *)
{
    // Make sure the scene is scaled in the view
    fitInView(0.0, 0.0, 200.0, 100.0, Qt::KeepAspectRatio);
}

#include "debugwidget.moc"
