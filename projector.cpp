#include "projector.h"

#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QScrollBar>

#include <algorithm>

#include "debug.h"

using std::min;
using std::max;

Projector::Projector(QWidget *parent)
    : QScrollArea(parent)
{
    // Grab the target widget
    _target = new QWidget(this);
    setWidget(_target);
    _target->show();

    // Initialize
    _loading0 = createLoadingWidget();
    _loading0->hide();
    _loading0->setParent(_target);
    _loading1 = createLoadingWidget();
    _loading1->hide();
    _loading1->setParent(_target);

    _page0 = new QLabel(_target);
    _page0->hide();
    _page1 = new QLabel(_target);
    _page1->hide();

    // Always expand
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);

    // No frame
    setFrameStyle(QFrame::NoFrame);

    // No scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Nothing displayed
    _placement[0] = QRect();
    _placement[1] = QRect();

    // Reset the scroll position when the ranges change
    connect(horizontalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(pagesChanged()));
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(pagesChanged()));
}

Projector::~Projector()
{
}

QWidget *Projector::createLoadingWidget()
{
    QWidget *loading = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout(loading);
    layout->setAlignment(Qt::AlignHCenter);

    layout->addStretch(1);

    QLabel *label = new QLabel("Loading", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    layout->addSpacing(20);

    QProgressBar *progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->setMaximumWidth(100);
    layout->addWidget(progress);

    layout->addStretch(1);

    return loading;
}

int Projector::heightForWidth(int width) const
{
    return width * 3 / 4;
}

QSize Projector::sizeHint() const
{
    return QSize(200, 100);
}

void Projector::showBlank()
{
    //debug()<<"Blank";
    _placement[0] = QRect();
    _placement[1] = QRect();
    _loading0->hide();
    _loading1->hide();
    _page0->hide();
    _page1->hide();
}

void Projector::showLoading0(const QRect &rect)
{
    //debug()<<"Loading0"<<rect;
    _placement[0] = rect;
    _loading0->setGeometry(rect);

    _page0->hide();
    _loading0->show();
    updateViewport();
}

void Projector::showLoading1(const QRect &rect)
{
    //debug()<<"Loading1"<<rect;
    _placement[1] = rect;
    _loading1->setGeometry(rect);

    _page1->hide();
    _loading1->show();
    updateViewport();
}

void Projector::showPage0(const QRect &rect, QPixmap image)
{
    //debug()<<"Page0"<<rect;
    _placement[0] = rect;
    _page0->setPixmap(image);
    _page0->setGeometry(rect);

    _loading0->hide();
    _page0->show();
    updateViewport();
}

void Projector::showPage1(const QRect &rect, QPixmap image)
{
    //debug()<<"Page1"<<rect;
    _placement[1] = rect;
    _page1->setPixmap(image);
    _page1->setGeometry(rect);

    _loading1->hide();
    _page1->show();
    updateViewport();
}

/**
 * @todo Should assert that this won't cause scaling
 */
void Projector::updatePosition0(const QRect &rect)
{
    _placement[0] = rect;
    _loading0->setGeometry(rect);
    _page0->setGeometry(rect);
    updateViewport();
}

void Projector::updatePosition1(const QRect &rect)
{
    _placement[1] = rect;
    _loading1->setGeometry(rect);
    _page1->setGeometry(rect);
    updateViewport();
}

void Projector::updateViewport()
{
    QRect area = displayArea();

    if (area.isValid())
    {
        // Shift the widgets if needed
        bool shifted = false;

        if (area.width() > viewport()->width())
        {
            // No width scrolling
            _placement[0].translate(-area.left(), 0);
            _placement[1].translate(-area.left(), 0);
            shifted = true;
        }
        else if (area.right() + area.left() > viewport()->width())
        {
            // Centre width
            int target = (viewport()->width() - area.width()) / 2;
            _placement[0].translate(-area.left() + target, 0);
            _placement[1].translate(-area.left() + target, 0);
            shifted = true;
        }

        if (area.height() > viewport()->height())
        {
            // No height scrolling
            _placement[0].translate(0, -area.top());
            _placement[1].translate(0, -area.top());
            shifted = true;
        }
        else if (area.bottom() + area.top() > viewport()->height())
        {
            // Centre height
            int target = (viewport()->height() - area.height()) / 2;
            _placement[0].translate(0, -area.top() + target);
            _placement[1].translate(0, -area.top() + target);
            shifted = true;
        }

        if (shifted)
        {
            // Reset the geometry of the widgets
            _loading0->setGeometry(_placement[0]);
            _page0->setGeometry(_placement[0]);
            _loading1->setGeometry(_placement[1]);
            _page1->setGeometry(_placement[1]);
            area = displayArea();
        }

        // Make the target widget stretch out to the edges of the pages plus the margins
        _target->resize(area.right() + area.left(), area.bottom() + area.top());
    }
}

void Projector::pagesChanged()
{
    QRect area = displayArea();

    // Far right
    QScrollBar *hBar = horizontalScrollBar();
    hBar->setValue(hBar->maximum());

    // Top
    QScrollBar *vBar = verticalScrollBar();
    vBar->setValue(vBar->minimum());
}

QRect Projector::displayArea()
{
    if (_placement[0].isValid() && _placement[1].isValid())
    {
        return _placement[0].united(_placement[1]);
    }
    else if (_placement[0].isValid())
    {
        return _placement[0];
    }
    else
    {
        return _placement[1];
    }
}

void Projector::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size() * 3 / 2);
}

void Projector::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

#include "projector.moc"
