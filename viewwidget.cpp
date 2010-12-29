#include "viewwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QLayout>

#include "steward.h"
#include "toolbarwidget.h"
#include "debug.h"

const int ViewWidget::SHADOW_WIDTH = 64;
const int ViewWidget::SHADOW_HEIGHT = 8;
const int ViewWidget::SHADOW_TONE = 0;
const int ViewWidget::SHADOW_FADE = 64;

ViewWidget::ViewWidget(Steward &steward, ToolbarWidget *toolbar, QWidget *parent)
    : QWidget(parent), _steward(steward), _shadow(SHADOW_WIDTH, SHADOW_HEIGHT, QImage::Format_ARGB32_Premultiplied)
{
    _toolbar = toolbar;

    // Make the shadow
    _shadow.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&_shadow);
    QLinearGradient gradient(0, 0, 0, SHADOW_HEIGHT);
    gradient.setColorAt(0.0, QColor(SHADOW_TONE, SHADOW_TONE, SHADOW_TONE, SHADOW_FADE));
    gradient.setColorAt(0.25, QColor(SHADOW_TONE, SHADOW_TONE, SHADOW_TONE, SHADOW_FADE / 2));
    gradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

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

int ViewWidget::toolbarHeight() const
{
    return parentWidget()->height() - height();
}

QSize ViewWidget::sizeHint() const
{
    return QSize(200, 100 - toolbarHeight());
}

int ViewWidget::heightForWidth(int width) const
{
    return (width * 3 + 2) / 4 - toolbarHeight();
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
            _toolbar->startShow();

            // Stop scrolling
            _steward.resetMouse();
        }
        else
        {
            // Hide the cursor
            setCursor(Qt::BlankCursor);

            // Hide the toolbar
            _toolbar->startHide();
        }
    }
    // Pass on other mouse presses
    else
    {
        event->ignore();
    }
}

void ViewWidget::resizeEvent(QResizeEvent *event)
{
    // Include the toolbar height in the total height
    QSize size = parentWidget()->size();

    // Pass on the new size to the steward
    _steward.setViewSize(size);
}

void ViewWidget::paintEvent(QPaintEvent *event)
{
    // Set up the painter
    QPainter painter(this);
    QRect updateRect = event->rect();

    // Paint under the toolbar
    int underHeight = toolbarHeight();
    painter.translate(QPoint(0, -underHeight));
    updateRect.translate(0, underHeight);

    // Get the steward to paint
    _steward.paintView(&painter, updateRect);

    // Paint on a shadow if needed
    if (underHeight > 0)
    {
        for (int x = 0; x < updateRect.right(); x += SHADOW_WIDTH)
        {
            painter.drawImage(x, underHeight, _shadow);
        }
    }
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Ignore if using the toolbar
    if (!_usingToolbar)
    {
        // Offset by toolbar height
        QPointF pos = event->posF();
        pos.ry() += toolbarHeight();

        // Tell the steward the mosue moved
        _steward.mouseMoved(pos);
    }
}

void ViewWidget::leaveEvent(QEvent* event)
{
    // Ignore if using the toolbar
    if (!_usingToolbar)
    {
        // Stop scrolling
        _steward.resetMouse();
    }
}

#include "viewwidget.moc"
