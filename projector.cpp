#include "projector.h"

#include <QPainter>
#include <QPaintEvent>
#include <QApplication>

#include "debug.h"

const double Projector::MAGNIFICATION = 1.0;
const double Projector::FRAMES_PER_SECOND = 60.0;

Projector::Projector(QWidget *parent)
    : QWidget(parent), _scroller(this), _loadingSprite(palette().color(QPalette::WindowText), palette().color(QPalette::Base))
{
    // Show nothing
    _isShown[0] = false;
    _isShown[1] = false;

    // Set the timer
    _refreshTimer.setInterval(int(1000.0 / FRAMES_PER_SECOND + 0.5));
    _refreshTimer.setSingleShot(true);
    connect(&_refreshTimer, SIGNAL(timeout()), SLOT(refresh()));

}

Projector::~Projector()
{
}

QSize Projector::sizeHint() const
{
    return QSize(200, 100);
}

int Projector::heightForWidth(int width) const
{
    return width * 3 / 4;
}

void Projector::showBlank()
{
    _isShown[0] = false;
    _isShown[1] = false;
    update();
}

void Projector::showLoading0(const QRect &rect)
{
    _isShown[0] = true;
    _isLoading[0] = true;
    _placement[0] = rect;
    update();
}

void Projector::showLoading1(const QRect &rect)
{
    _isShown[1] = true;
    _isLoading[1] = true;
    _placement[1] = rect;
    update();
}

void Projector::showPage0(const QRect &rect, QPixmap pixmap)
{
    Q_ASSERT(rect.size() == pixmap.size());

    _isShown[0] = true;
    _isLoading[0] = false;
    _placement[0] = rect;
    _pageSprite0.setPixmap(pixmap);
    update();
}

void Projector::showPage1(const QRect &rect, QPixmap pixmap)
{
    Q_ASSERT(rect.size() == pixmap.size());

    _isShown[1] = true;
    _isLoading[1] = false;
    _placement[1] = rect;
    _pageSprite1.setPixmap(pixmap);
    update();
}

void Projector::updatePosition0(const QRect &rect)
{
    _placement[0] = rect;
    update();
}

void Projector::updatePosition1(const QRect &rect)
{
    _placement[1] = rect;
    update();
}

void Projector::pagesChanged()
{
    _scroller.reset();
}

void Projector::resizeEvent(QResizeEvent *event)
{
    // Expand the size
    QSize newSize = (QSizeF(event->size()) * MAGNIFICATION).toSize();

    // Tell the scroller
    _scroller.setExtent(newSize);

    // Tell the steward
    emit resized(newSize);
}

void Projector::paintEvent(QPaintEvent *event)
{
    // Set up the painter
    QPainter painter(this);
    QRect updateRect = event->rect();

    // Both shown
    if (_isShown[0] && _isShown[1])
    {
        if (_isLoading[0] && _isLoading[1])
        {
            // Both loading
            _loadingSprite.setGeometry(_placement[0] | _placement[1]);
            _loadingSprite.paint(&painter, updateRect);
        }
        else if (_isLoading[0] && !_isLoading[1])
        {
            // First loading
            _loadingSprite.setGeometry(_placement[0]);
            _loadingSprite.paint(&painter, updateRect);

            // Second shown
            _pageSprite1.setTopLeft(_placement[1].topLeft());
            _pageSprite1.paint(&painter, updateRect);
        }
        else if (!_isLoading[0] && _isLoading[1])
        {
            // First shown
            _pageSprite0.setTopLeft(_placement[0].topLeft());
            _pageSprite0.paint(&painter, updateRect);

            // Second loading
            _loadingSprite.setGeometry(_placement[1]);
            _loadingSprite.paint(&painter, updateRect);
        }
        else if (!_isLoading[0] && !_isLoading[1])
        {
            // Both shown
            _pageSprite0.setTopLeft(_placement[0].topLeft());
            _pageSprite0.paint(&painter, updateRect);
            _pageSprite1.setTopLeft(_placement[1].topLeft());
            _pageSprite1.paint(&painter, updateRect);
        }
    }
    // One shown
    else if (_isShown[0])
    {
        if (_isLoading[0])
        {
            // Loading
            _loadingSprite.setGeometry(_placement[0]);
            _loadingSprite.paint(&painter, updateRect);
        }
        else
        {
            // Shown
            _pageSprite0.setTopLeft(_placement[0].topLeft());
            _pageSprite0.paint(&painter, updateRect);
        }
    }

    // Refresh if doing loading animation
    if ((_isShown[0] && _isLoading[0]) || (_isShown[1] && _isLoading[1]))
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
    repaint();
}

void Projector::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

#include "projector.moc"
