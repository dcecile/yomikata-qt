#include "projector.h"

#include <QPalette>
#include <QPainter>
#include <QApplication>

#include "debug.h"
#include "displaymetrics.h"

const double Projector::MAGNIFICATION = 2.0;
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

void Projector::clear(const DisplayMetrics &displayMetrics)
{
    // Reset the scroller to the pages start with the new extent
    _scroller.reset(displayMetrics.slack);

    // Set to pure blank
    _isShown[0] = false;
    _isShown[1] = false;
    _isLoading[0] = true;
    _isLoading[1] = true;

    // Set up the loading rects
    update(displayMetrics, QPixmap(), QPixmap());
}

void Projector::update(const DisplayMetrics &displayMetrics, const QPixmap &pixmap0, const QPixmap &pixmap1)
{
    const QPixmap *pixmap[] = {&pixmap0, &pixmap1};

    for (int i = 0; i < 2; i++)
    {
        if (!displayMetrics.pages[i].isNull())
        {
            // Update the position
            _isShown[i] = true;
            _placement[i] = displayMetrics.pages[i];

            if (!pixmap[i]->isNull())
            {
                // Pixmap loaded
                _isLoading[i] = false;
                _pageSprite[i].setPixmap(*pixmap[i]);
            }
        }
    }

    // Schedule a repaint
    emit update();
}

bool Projector::tryUpdate(const DisplayMetrics &displayMetrics)
{
    bool failed = false;
    bool updateNeeded = false;

    for (int i = 0; !failed && i < 2; i++)
    {
        if (!displayMetrics.pages[i].isNull())
        {
            // Has to match
            Q_ASSERT(_isShown[i]);

            // No problem loading size changes
            // Or displayed position changes
            if (_isLoading[i] || _placement[i].size() == displayMetrics.pages[i].size())
            {
                if (_placement[i] != displayMetrics.pages[i])
                {
                    _placement[i] = displayMetrics.pages[i];
                    updateNeeded = true;
                }
            }
            else
            {
                failed = true;
            }
        }
    }

    if (failed)
    {
        clear(displayMetrics);
    }
    else
    {
        // Update the scroller's bounds because the total width maybe changed
        _scroller.update(displayMetrics.slack);
    }

    if (failed || updateNeeded)
    {
        emit update();
    }

    return !failed;
}

void Projector::setViewSize(const QSize &size)
{
    _viewSize = size;
    _fullSize = (QSizeF(_viewSize) * MAGNIFICATION).toSize();
}

QSize Projector::viewSize() const
{
    return _viewSize;
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
        for (int i = 0; i < 2; i++)
        {
            if (_isShown[i])
            {
                if (_isLoading[i])
                {
                    // Loading
                    _loadingSprite.setGeometry(scrolled[i]);
                    _loadingSprite.paint(painter, updateRect);
                }
                else
                {
                    // Shown
                    _pageSprite[i].setTopLeft(scrolled[i].topLeft());
                    _pageSprite[i].paint(painter, updateRect);
                }
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
