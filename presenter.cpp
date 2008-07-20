#include "presenter.h"

#include <QPaintEvent>
#include <QPainter>
#include <KApplication>
#include <KDebug>

Presenter::Presenter(Lens &lensA, Lens &lensB, QWidget *parent)
    : QWidget(parent), _lensA(lensA), _lensB(lensB)
{
    _mangaMode = true;
    _resizeTime.start();
    _resizePainted = true;
}

Presenter::~Presenter()
{
}

void Presenter::paintEvent(QPaintEvent *event)
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
    _lensA.paint(&painter, event->rect());
    _lensB.paint(&painter, event->rect());

    if (!_resizePainted) {
        kDebug()<<"Resize turnover"<<_resizeTime.elapsed()<<"ms";
        _resizePainted = true;
    }
}

void Presenter::resizeEvent(QResizeEvent *)
{
    if (_resizePainted) {
        _resizeTime.restart();
        _resizePainted = false;
    } else {
        kDebug()<<"Resize not painted"<<_resizeTime.elapsed()<<"ms";
    }
    emit displayAreaChanged(size());
}

#include "presenter.moc"
