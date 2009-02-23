#include "projector.h"

#include <QPalette>
#include <QPainter>
#include <QApplication>

#include "debug.h"
#include "displaymetrics.h"

const double Projector::MAGNIFICATION = 1.0;
const double Projector::FRAMES_PER_SECOND = 60.0;

Projector::Projector(QObject *parent)
    : QObject(parent), _scroller(this), _loadingSprite(QPalette().color(QPalette::WindowText), QPalette().color(QPalette::Base))
{
    // Show nothing
    _isShown[0] = false;
    _isShown[1] = false;

    // Connect to the scroller
    connect(&_scroller, SIGNAL(enableRefresh(bool)), SLOT(enableRefresh(bool)));

    // Set the timer
    _refreshTimer.setInterval(int(1000.0 / FRAMES_PER_SECOND + 0.5));
    _refreshTimer.setSingleShot(true);
    connect(&_refreshTimer, SIGNAL(timeout()), SLOT(refresh()));
}

Projector::~Projector()
{
}

void Projector::setDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1)
{
    // Reset the scroller to the pages start with the new extent
    _scroller.reset(displayMetrics.slack);

    // Set to blank / loading
    _isShown[0] = false;
    _isShown[1] = false;
    _isLoading[0] = true;
    _isLoading[1] = true;
    //debug()<<"Reset";

    // Update to the current
    updateDisplay(displayMetrics, pixmap0, pixmap1);
}

void Projector::updateDisplay(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1)
{
    if (!displayMetrics.pages[0].isNull())
    {
        // Update the position
        _isShown[0] = true;
        _placement[0] = displayMetrics.pages[0];

        if (!pixmap0.isNull())
        {
            // Pixmap loaded
            //debug()<<"Pixmap 0"<<_isLoading[1];
            _isLoading[0] = false;
            _pageSprite0.setPixmap(pixmap0);
        }
    }

    if (displayMetrics.pages[1].isValid())
    {
        // Update the position
        _isShown[1] = true;
        _placement[1] = displayMetrics.pages[1];

        if (!pixmap1.isNull())
        {
            // Pixmap loaded
            //debug()<<"Pixmap 1"<<_isLoading[0];
            _isLoading[1] = false;
            _pageSprite1.setPixmap(pixmap1);
        }
    }

    // Schedule a repaint
    emit update();
}

void Projector::retrieveDisplay(QRect *rect0, QRect *rect1)
{
    if (_isShown[0])
    {
        *rect0 = _placement[0];
    }
    else
    {
        *rect0 = QRect();
    }

    if (_isShown[1])
    {
        *rect1 = _placement[1];
    }
    else
    {
        *rect1 = QRect();
    }
}

void Projector::setViewSize(const QSize &size)
{
    _viewSize = size;
    _fullSize = (QSizeF(_viewSize) * MAGNIFICATION).toSize();
}

QSize Projector::fullSize() const
{
    return _fullSize;
}

void Projector::paint(QPainter *painter, const QRect &updateRect)
{
    // Calculate the scrolled rects
    QPoint scrolling = _scroller.position();
    QRect scrolled[2];

    scrolled[0] = _placement[0].translated(-scrolling);
    scrolled[1] = _placement[1].translated(-scrolling);

    //debug()<<"Position"<<scrolling;

    if (_isShown[0] && _isShown[1] && _isLoading[0] && _isLoading[1])
    {
        // Both loading
        _loadingSprite.setGeometry(scrolled[0] | scrolled[1]);
        _loadingSprite.paint(painter, updateRect);
    }
    else
    {
        if (_isShown[0])
        {
            if (_isLoading[0])
            {
                // First loading
                _loadingSprite.setGeometry(scrolled[0]);
                _loadingSprite.paint(painter, updateRect);
            }
            else
            {
                // First shown
                _pageSprite0.setTopLeft(scrolled[0].topLeft());
                _pageSprite0.paint(painter, updateRect);
            }
        }

        if (_isShown[1])
        {
            if (_isLoading[1])
            {
                // Second loading
                _loadingSprite.setGeometry(scrolled[1]);
                _loadingSprite.paint(painter, updateRect);
            }
            else
            {
                // Second shown
                _pageSprite1.setTopLeft(scrolled[1].topLeft());
                _pageSprite1.paint(painter, updateRect);
            }
        }
    }

    // Refresh if scrolling or doing loading animation
    if (_refreshRequested || (_isShown[0] && _isLoading[0]) || (_isShown[1] && _isLoading[1]))
    {
        _refreshTimer.start();
    }
}

void Projector::refresh()
{
    // Pump the events, to make sure input gets handled
    QApplication::syncX();
    QApplication::processEvents();

    // Repaint immediately
    emit repaint();
}

void Projector::enableRefresh(bool enable)
{
    _refreshRequested = enable;

    // If on and not running, update immediately (repeat after)
    if (_refreshRequested && !_refreshTimer.isActive())
    {
        emit update();
    }
}

void Projector::mouseMoved(const QPointF &pos)
{
    _scroller.mouseMoved(pos);
}

#include "projector.moc"
