#include "viewwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

#include "steward.h"

ViewWidget::ViewWidget(Steward &steward, QWidget *parent)
    : QWidget(parent), _steward(steward)
{
    // Subscribe to mouse movement
    setMouseTracking(true);

    // Hide the cursor
    setCursor(Qt::BlankCursor);

    // Connect to refresh signals
    connect(&_steward, SIGNAL(viewUpdate()), SLOT(update()));
    connect(&_steward, SIGNAL(viewRepaint()), SLOT(repaint()));
}

ViewWidget::~ViewWidget()
{
}

QSize ViewWidget::sizeHint() const
{
    return QSize(200, 100);
}

int ViewWidget::heightForWidth(int width) const
{
    return (width * 3 + 2) / 4;
}

void ViewWidget::resizeEvent(QResizeEvent *event)
{
    // Pass on the new size to the steward
    _steward.setViewSize(event->size());
}

void ViewWidget::paintEvent(QPaintEvent *event)
{
    // Set up the painter
    QPainter painter(this);
    QRect updateRect = event->rect();

    // Get the steward to paint
    _steward.paintView(&painter, updateRect);
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Tell the steward the mosue moved
    _steward.mouseMoved(event->posF());
}

#include "viewwidget.moc"
