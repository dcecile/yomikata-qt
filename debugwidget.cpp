#include "debugwidget.h"

#include <QMouseEvent>

#include "debug.h"

DebugWidget::DebugWidget(Book &book, QWidget *parent)
    : QGraphicsView(parent), _book(book)
{
    // Create the scene
    _scene.setItemIndexMethod(QGraphicsScene::NoIndex);

    // Create the pages
    setup();

    // Subscribe to changes
    connect(&_book, SIGNAL(changed()), SLOT(setup()));

    // Create the view
    setScene(&_scene);
    setRenderHint(QPainter::Antialiasing);
    setCacheMode(QGraphicsView::CacheBackground);
    setDragMode(QGraphicsView::NoDrag);
    setInteractive(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // No need to expand
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMaximumHeight(100);
}

DebugWidget::~DebugWidget()
{
}

void DebugWidget::setup()
{
    // Empty the scene
    _scene.clear();
    _pages.clear();

    // Don't do anything if there's no pages
    if (_book.numPages() == 0)
    {
        return;
    }

    // Create pages
    int pos = 0;
    int current0 = _book.page0();
    int current1 = _book.page1();
    int offset;
    bool dual;

    for (int i = 0; i < _book.numPages(); i++)
    {
        offset = _book.pairedPageOffset(i);
        dual = _book.isDual(i);
        pos -= 60 + 5 * offset + 50 * dual;
        DebugPageItem *temp = new DebugPageItem(pos, 10,
                (i == current0) || (i == current1), dual, i);
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

    // Center on the active one
    centerOn(_pages[current0]);
}

QSize DebugWidget::sizeHint() const
{
    return QSize(300, 50);
}

void DebugWidget::resizeEvent(QResizeEvent *)
{
    // Make sure the scene is scaled in the view
    fitInView(0.0, 0.0, 400.0, 100.0, Qt::KeepAspectRatio);
}

void DebugWidget::mousePressEvent(QMouseEvent *event)
{
    event->ignore();
}

#include "debugwidget.moc"
