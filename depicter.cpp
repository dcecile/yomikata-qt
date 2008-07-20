#include "depicter.h"

#include <QThread>
#include <QPaintEvent>
#include <QPainter>
#include <KDebug>
#include <KApplication>

#include "page.h"

Depicter::Depicter(QWidget *parent)
    : QWidget(parent), _lensOne(this), _lensTwo(this)
{
    _pageOne = NULL;
    _pageTwo = NULL;
    _twoPages = false;

    connect(&_lensOne, SIGNAL(update(const QRect &)), this, SLOT(updateLensArea(const QRect &)));
    connect(&_lensTwo, SIGNAL(update(const QRect &)), this, SLOT(updateLensArea(const QRect &)));

    _resizeTime.start();
    _resizePainted = true;
}

Depicter::~Depicter()
{
}

void Depicter::depict(Page *pageOne, Page *pageTwo)
{
    // Disconnect from old pages
    if (_pageOne != NULL) {
        disconnect(_pageOne, SIGNAL(pixmapUpdated(Page *)), this, SLOT(pixmapUpdated(Page *)));
    }
    if (_pageTwo != NULL) {
        disconnect(_pageTwo, SIGNAL(pixmapUpdated(Page *)), this, SLOT(pixmapUpdated(Page *)));
    }

    // Set new pages
    _pageOne = pageOne;
    _pageTwo = pageTwo;
    _twoPages = _pageTwo != NULL;

    // Connect
    connect(_pageOne, SIGNAL(pixmapUpdated(Page *)), this, SLOT(pixmapUpdated(Page *)));
    if (_pageTwo != NULL) {
        connect(_pageTwo, SIGNAL(pixmapUpdated(Page *)), this, SLOT(pixmapUpdated(Page *)));
    }

    // Calibrate the lenses
    if (_pageOne->isCached()) {
        _lensOne.focus(_pageOne->getPixmap(), _pageOne->getThumbnail(), _pageOne->getTargetRect());
    } else {
        _lensOne.wait(_pageOne->getPageNumber() + 1, _pageOne->getTargetRect());
    }

    if (_twoPages) {
        if (_pageTwo->isCached()) {
            _lensTwo.focus(_pageTwo->getPixmap(), _pageTwo->getThumbnail(), _pageTwo->getTargetRect());
        } else {
            _lensTwo.wait(_pageTwo->getPageNumber() + 1, _pageTwo->getTargetRect());
        }
    } else {
        _lensTwo.close();
    }

    // Update
    update();
}

void Depicter::currentPagesReplanned()
{
    //kDebug()<<QThread::currentThreadId();
    if (_pageOne != NULL) {
        _lensOne.reposition(_pageOne->getTargetRect());
        if (_twoPages) {
            _lensTwo.reposition(_pageTwo->getTargetRect());
        }
    }
}

void Depicter::pixmapUpdated(Page *page)
{
    // Focus on the new pixmap
    if (page == _pageOne) {
        _lensOne.focus(_pageOne->getPixmap(), _pageOne->getThumbnail(), _pageOne->getTargetRect());
    } else {
        Q_ASSERT(page == _pageTwo);
        _lensTwo.focus(_pageTwo->getPixmap(), _pageTwo->getThumbnail(), _pageTwo->getTargetRect());
    }

    // Update
    update();
}

void Depicter::paintEvent(QPaintEvent *event)
{
    // Make sure the previous paint is ACTUALLY FINISHED
    QTime syncTime;
    syncTime.start();
    KApplication::kApplication()->syncX();
    int elapsedSync = syncTime.elapsed();
    if (elapsedSync > 50) {
        kDebug()<<"Sync time"<<elapsedSync<<"ms";
    }

    // Create the painter
    QPainter painter(this);

    //kDebug()<<"Painting (hints"<<painter.renderHints()<<")";

    // Have the lenses paint
    _lensOne.paint(&painter, event->rect());
    _lensTwo.paint(&painter, event->rect());

    if (!_resizePainted) {
        kDebug()<<"Resize turnover"<<_resizeTime.elapsed()<<"ms";
        _resizePainted = true;
    }
}

void Depicter::resizeEvent(QResizeEvent *)
{
    if (_resizePainted) {
        _resizeTime.restart();
        _resizePainted = false;
    } else {
        kDebug()<<"Resize not painted"<<_resizeTime.elapsed()<<"ms";
    }
    emit displayAreaChanged(size());

    // Update the planned zones
    currentPagesReplanned();
}

void Depicter::updateLensArea(const QRect &zone)
{
    update(zone);
}

#include "depicter.moc"
