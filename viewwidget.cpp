#include "viewwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

#include "steward.h"

ViewWidget::ViewWidget(Steward &steward, QWidget *parent)
    : QWidget(parent), _steward(steward)
{
    // Use a darker background colour
    QPalette newPalette = palette();
    QColor color = newPalette.color(QPalette::Window);
    qreal value = color.valueF() + (value > 0.5 ?0.02 :-0.02);
    color.setHsvF(color.hueF(), color.saturationF(), value);
    newPalette.setColor(QPalette::Window, color);
    setPalette(newPalette);
    setAutoFillBackground(true);

    // Subscribe to mouse movement
    setMouseTracking(true);

    // Hide the cursor
    setCursor(Qt::BlankCursor);

    // Connect to refresh signals
    connect(&_steward, SIGNAL(viewUpdate()), SLOT(update()));
    connect(&_steward, SIGNAL(viewRepaint()), SLOT(repaint()));

    // Start reading, not using toolbar
    _usingToolbar = false;
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

void ViewWidget::mousePressEvent(QMouseEvent *event)
{
    // Enable/disable toolbar
    if (event->button() == Qt::LeftButton)
    {
        _usingToolbar = !_usingToolbar;

        if (_usingToolbar)
        {
            // Show the cursor
            setCursor(Qt::ArrowCursor);

            // Show the toolbar
            emit showToolbar();
        }
        else
        {
            // Hide the cursor
            setCursor(Qt::BlankCursor);

            // Hide the toolbar
            emit hideToolbar();
        }
    }
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
    // Ignore if using the toolbar
    if (!_usingToolbar)
    {
        // Tell the steward the mosue moved
        _steward.mouseMoved(event->posF());
    }
}

#include "viewwidget.moc"
