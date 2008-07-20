#include "lens.h"

#include <QLinearGradient>
#include <KDebug>
#include <KLocalizedString>

Lens::Lens(QObject *parent)
    : QObject(parent), _loadingTimeLine(ANIM_PERIOD)
{
    _state = Closed;
    connect(&_loadingTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(loadingAnimation(qreal)));
    _loadingTimeLine.setLoopCount(0);
    _loadingTimeLine.setUpdateInterval(100);
    _loadingTimeLine.setCurveShape(QTimeLine::LinearCurve);
    _resizeTime.start();
}

Lens::~Lens()
{
}

void Lens::close()
{
    // Close the lens
    // Stop displaying anything
    _state = Closed;
    _pixmap = QPixmap();
    _loadingTimeLine.stop();
}

void Lens::wait(int pageNum, const QRect &displayRect)
{
    // Wait for the page to be loaded
    _state = Waiting;
    _pageNum = pageNum;
    _displayRect = displayRect;
    _animationValue = 0.0;
    _pixmap = QPixmap();
    _loadingTimeLine.start();

    //kDebug()<<_pageNum<<_displayRect;
}

void Lens::focus(QPixmap pixmap, QPixmap thumbnail, const QRect &displayRect)
{
    // Display the page
    _state = Focused;
    _pixmap = pixmap;
    _thumbnail = thumbnail;
    _displayRect = displayRect;
    _loadingTimeLine.stop();
}

void Lens::reposition(const QRect &displayRect)
{
    Q_ASSERT(_state != Closed);

    // Remember the old rect
    QRect oldRect = _displayRect;

    // Move the display rect
    _displayRect = displayRect;

    // If there's a change
    if (oldRect != _displayRect) {
        // Update both rects
        emit update(oldRect.united(_displayRect));
    }
}

void Lens::paint(QPainter *painter, const QRect &)
{
    switch (_state) {
        case Closed:
            return;
            break;
        case Waiting:
            paintWaiting(painter);
            break;
        case Focused:
            paintFocused(painter);
            break;
        default:
            Q_ASSERT(false);
    }
}

void Lens::paintWaiting(QPainter *painter)
{
    //kDebug()<<"Painting";

    // Draw a blank page
    painter->save();
    painter->setPen(QPen(QColor("#e0e0e0"), 1.5));
    painter->setBrush(QColor("#eaeaea"));
    QRect pageRect(_displayRect);
    pageRect.adjust(2,2,-2,-2);
    painter->drawRect(pageRect);
    painter->restore();

    // Show the page number
    QRect textRect(_displayRect);
    textRect.adjust(0,int(_displayRect.height()*0.30),0,int(-_displayRect.height()*0.60));
    painter->drawText(textRect, Qt::AlignCenter, i18n("Page %1", _pageNum));

    // Show the loading progress
    painter->save();
    painter->setPen(QPen(QColor("#909090"), 1));
    QRect loadingRect(_displayRect);
    loadingRect.adjust(
            int(_displayRect.width()*0.35),
            int(_displayRect.height()*0.40),
            int(-_displayRect.width()*0.35),
            int(-_displayRect.height()*0.55));
    QLinearGradient loadingGradient(loadingRect.bottomLeft(), loadingRect.topRight());
    QColor colorA("red"), colorB("yellow");
    int h, s, v;
    colorA.getHsv(&h, &s, &v);
    h = (h + int(360.0 * _animationValue + 0.5)) % 360;
    colorA.setHsv(h, s, v);
    colorB.getHsv(&h, &s, &v);
    h = (h + int(360.0 * _animationValue + 0.5)) % 360;
    colorB.setHsv(h, s, v);
    loadingGradient.setColorAt(0.0, colorA);
    loadingGradient.setColorAt(1.0, colorB);
    painter->setBrush(loadingGradient);
    painter->drawRect(loadingRect);
    painter->restore();
}

void Lens::loadingAnimation(qreal value)
{
    _animationValue = value;
    QRect loadingRect(_displayRect);
    loadingRect.adjust(
            int(_displayRect.width()*0.35),
            int(_displayRect.height()*0.40),
            int(-_displayRect.width()*0.35),
            int(-_displayRect.height()*0.55));
    emit update(loadingRect);
}

void Lens::paintFocused(QPainter *painter)
{
    // Check if width is small by a small amount
    if (_displayRect.height() == _pixmap.height() && _displayRect.width() != _pixmap.width()) {

        Q_ASSERT(_displayRect.width() == (_pixmap.width() + 1) || _displayRect.width() == (_pixmap.width() - 1));

        ///@todo Fix off-by-one width
        //kDebug()<<"width off by one";
        QRect dest(_displayRect);
        dest.setWidth(_pixmap.width());
        painter->drawPixmap(_displayRect, _pixmap);


    } else if (_displayRect.size() != _pixmap.size()) {
        //kDebug()<<"display"<<_displayRect<<"decoded"<<_pixmap.size();
        _resizeTime.restart();
        //QPixmap temp = _pixmap.scaled(_displayRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation); painter->drawPixmap(_displayRect.topLeft(), temp);
        painter->drawPixmap(_displayRect, _thumbnail);//_pixmap);
        //kDebug()<<"resize time"<<_resizeTime.elapsed();
    } else {
        painter->drawPixmap(_displayRect.topLeft(), _pixmap);
    }
}

#include "lens.moc"
