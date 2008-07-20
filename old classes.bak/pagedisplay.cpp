#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <KDebug>
#include <QTime>
#include <QTextStream>
#include <KApplication>

#include "pagedisplay.h"

#include "simplescroller.h"
#include "inertiascroller.h"

const qreal PageDisplay::ZOOM_MODES[] = {1.2, 2.0, 3.0, -1.0};

PageDisplay::PageDisplay(bool mangaMode, QWidget *parent)
    :QWidget(parent), _mangaMode(mangaMode)
{
    _displaying = false;
    _zoomEnabled = false;
    _zoomIndex = 1;
    _boundingSize = _displaySize = size();

    _pageNumberLabel = new QLabel(this);
    _pageNumberLabel->hide();
    _pageNumberLabel->setBackgroundRole(QPalette::Window);
    _pageNumberLabel->setAutoFillBackground(true);
    _pageNumberLabel->setAlignment(Qt::AlignCenter);


    _zoomLabel = new QLabel(this);
    _zoomLabel->hide();
    _zoomLabel->setBackgroundRole(QPalette::Window);
    _zoomLabel->setAutoFillBackground(true);
    _zoomLabel->setAlignment(Qt::AlignCenter);

    _displayScroller = new InertiaScroller(this);
    //_displayScroller = new SimpleScroller(this);
    connect(_displayScroller, SIGNAL(moved(const QPoint&)), this, SLOT(moveDisplay(const QPoint&)));

    setMouseTracking(true);

    _mouseOver = false;
}

void PageDisplay::loadingStarted()
{
    changeCursor(Qt::WaitCursor);
}

void PageDisplay::setMangaMode(bool enabled)
{
    Q_ASSERT(false);
    _mangaMode = enabled;
}

void PageDisplay::setOnePage(const QPixmap &pixmap, int pageNum, int totalPages)
{
    // Make sure no painting gets done while the pages are changing
    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    // If this is the first page being displayed, start the mouse idle timer and enable zoom
    if (!_displaying) {
        _displaying = true;
        mouseMoved();
        emit zoomToggleEnabled(true);
    }

    // There is a new pixmap to display
    _pixmap[0] = pixmap;
    _scaledPixmap[0] = QPixmap();
    _doFullScale[0] = false;
    _pixmap[1] = QPixmap();
    _singlePage = true;

    // Position everything correctly
    adjustLayout();

    // Make sure the page number is shown
    setPageNumber(pageNum, -1, totalPages);

    // Restart the timer to hide the page number
    const int PAGE_NUMBER_HIDE_DELAY = 1500;
    _pageNumberTimer.stop();
    _pageNumberTimer.start(PAGE_NUMBER_HIDE_DELAY, this);

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::setTwoPages(const QPixmap &pixmapA, const QPixmap &pixmapB, int pageNumA, int pageNumB, int totalPages)
{
    // Make sure no painting gets done while the pages are changing
    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    // If these are the first pages being displayed, start the mouse idle timer and enable zoom
    if (!_displaying) {
        _displaying = true;
        mouseMoved();
        emit zoomToggleEnabled(true);
    }

    // There are new pixmaps to display
    if (_mangaMode) {
        _pixmap[1] = pixmapA;
        _pixmap[0] = pixmapB;
    } else {
        _pixmap[0] = pixmapA;
        _pixmap[1] = pixmapB;
    }
    _scaledPixmap[0] = QPixmap();
    _scaledPixmap[1] = QPixmap();
    _doFullScale[0] = false;
    _doFullScale[1] = false;
    _singlePage = false;

    // Position everything correctly
    adjustLayout();

    // Make sure the page number is shown
    setPageNumber(pageNumA, pageNumB, totalPages);

    // Restart the timer to hide the page number
    const int PAGE_NUMBER_HIDE_DELAY = 1500;
    _pageNumberTimer.stop();
    _pageNumberTimer.start(PAGE_NUMBER_HIDE_DELAY, this);

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::setPageNumber(int pageNumA, int pageNumB, int totalPages)
{
    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    // Stop the timer to hide the page number
    _pageNumberTimer.stop();

    // Show the page number
    const int WIDTH_PADDING = 20;
    QString text;
    if (pageNumB == -1) {
        text = QString("%1 / %2").arg(QString::number(pageNumA + 1), QString::number(totalPages));
    } else {
        text = QString("%1-%2 / %3").arg(QString::number(pageNumA + 1), QString::number(pageNumB + 1), QString::number(totalPages));
    }
    QFontMetrics metrics(_pageNumberLabel->font());
    _pageNumberLabel->resize(metrics.width(text) + WIDTH_PADDING, _pageNumberLabel->height());
    _pageNumberLabel->setText(text);
    _pageNumberLabel->move(width() - _pageNumberLabel->width(), height() - _pageNumberLabel->height());
    _pageNumberLabel->raise();
    _pageNumberLabel->show();

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::toggleZoom()
{
    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    // Assert something's being displayed
    Q_ASSERT(_displaying);

    _zoomEnabled = !_zoomEnabled;
    _newMousePath = true;

    if (_zoomEnabled) {
        changeCursor(Qt::BlankCursor);
    } else {
        mouseMoved();
    }

    if (!_zoomEnabled) {
        // Disable the scroller
        _displayScroller->stop();
    }

    // Update action enabled statuses
    if (!_zoomEnabled) {
        emit zoomInEnabled(false);
        emit zoomOutEnabled(false);
    } else {
        updateZoomStatus();
    }

    adjustLayout();

    update();

    // Tell the loader that the display size is changing
    emit displaySizeChanged(_boundingSize);

    if (_zoomEnabled) {
        // Show the zoom label
        showZoomLabel();
    } else {
        // Immediately hide the zoom label
        _zoomTimer.stop();
        _zoomLabel->hide();
    }

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::zoomIn()
{
    Q_ASSERT(_zoomEnabled && ZOOM_MODES[_zoomIndex + 1] != -1.0);

    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    _zoomIndex++;

    updateZoomStatus();

    adjustLayout();

    update();

    // Tell the loader that the display size is changing
    emit displaySizeChanged(_boundingSize);

    kDebug()<<"Zoom "<<_boundingSize<<endl;

    // Show the zoom label
    showZoomLabel();

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::zoomOut()
{
    Q_ASSERT(_zoomEnabled && _zoomIndex > 0);

    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    _zoomIndex--;

    updateZoomStatus();

    adjustLayout();

    update();

    // Tell the loader that the display size is changing
    emit displaySizeChanged(_boundingSize);

    kDebug()<<"Zoom "<<_boundingSize<<endl;

    // Show the zoom label
    showZoomLabel();

    setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::updateZoomStatus()
{
    emit zoomInEnabled(ZOOM_MODES[_zoomIndex + 1] != -1.0);
    emit zoomOutEnabled(_zoomIndex > 0);
}

void PageDisplay::showZoomLabel()
{
    // Stop the timer to hide the zoom label
    _zoomTimer.stop();

    // Show the zoom label
    const int WIDTH_PADDING = 20;
    QString text;
    QTextStream textStream(&text);
    textStream.setRealNumberNotation(QTextStream::FixedNotation);
    textStream.setRealNumberPrecision(1);
    textStream<<ZOOM_MODES[_zoomIndex]<<'x';
    QFontMetrics metrics(_zoomLabel->font());
    _zoomLabel->resize(metrics.width(text) + WIDTH_PADDING, _pageNumberLabel->height());
    _zoomLabel->setText(text);
    _zoomLabel->move(width() - _zoomLabel->width(), 0);
    _zoomLabel->raise();
    _zoomLabel->show();
}
void PageDisplay::hideZoomLabel()
{
    // Restart the timer to hide the zoom label
    const int ZOOM_LABEL_HIDE_DELAY = 1500;
    _zoomTimer.stop();
    _zoomTimer.start(ZOOM_LABEL_HIDE_DELAY, this);
}

void PageDisplay::paintEvent(QPaintEvent *event)
{
    // Make sure we have something to paint
    if (!_displaying) {
        return;
    }

    // Make sure the previous paint is ACTUALLY FINISHED
    KApplication::kApplication()->syncX();

    //kDebug()<<"Painting "<<event->rect()<<endl;

    //QTime time;time.start();

    QPainter painter(this);
    //painter.setRenderHint(QPainter::SmoothPixmapTransform );

    for (int i=0; i<(_singlePage?1:2); i++) {
        QRect paintRect(event->rect().intersected(_destRect[i]));

        if (paintRect.isEmpty()) {
            continue;
        }

        if (_prescaled[i]) {
            if (paintRect == _destRect[i] && !_zoomEnabled) {
                // Paint everything
                painter.drawPixmap(_destRect[i].topLeft(), _pixmap[i]);
            } else {
                // Move the source rectangle up and left compared to the destination rectangle
                QPoint delta = paintRect.topLeft() - _destRect[i].topLeft();
                painter.drawPixmap(paintRect, _pixmap[i], QRect(delta, paintRect.size()));
            }
        } else {
            /*const bool PRE_FAST_SCALE = false;
            if (PRE_FAST_SCALE && _scaledPixmap[i].isNull() && !_doFullScale[i]) {

                // Move the source rectangle up and left compared to the destination rectangle
                //  and scale it to the size of the original pixmap
                QPoint delta = paintRect.topLeft() - _destRect[i].topLeft();
                qreal scaleFactor = qreal(_pixmap[i].height()) / qreal(_destRect[i].height());
                QRect sourceRect(delta * scaleFactor, paintRect.size() * scaleFactor);

                // Do a fast scale of the image
                painter.drawPixmap(paintRect, _pixmap[i], sourceRect);

                kDebug()<<"Unscaled "<<QRect(delta, paintRect.size())<<"scaled "<<QRect(delta * scaleFactor, paintRect.size() * scaleFactor)<<endl;
                // Schedule a full scale
                //_doFullScale[i] = true;
                //update();

            } else {
                if ((!PRE_FAST_SCALE && _scaledPixmap[i].isNull())|| (PRE_FAST_SCALE && _doFullScale[i])) {
            }*/
            if (!_doFullScale[i]) {
                // Scale the pixmap up/down for the first time
                //kDebug()<<"Scaling"<<endl;
                _scaledPixmap[i] = _pixmap[i].scaled(_destRect[i].size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                _doFullScale[i] = true;
            }

            if (paintRect == _destRect[i] && !_zoomEnabled) {
                // Paint everything
                painter.drawPixmap(_destRect[i].topLeft(), _scaledPixmap[i]);
            } else {
                // Move the source rectangle up and left compared to the destination rectangle
                QPoint delta = paintRect.topLeft() - _destRect[i].topLeft();
                painter.drawPixmap(paintRect, _scaledPixmap[i], QRect(delta, paintRect.size()));
            }

            // Debug: show that a page isn't loaded correctly
            painter.setPen(Qt::red);
            painter.drawLine(paintRect.topLeft(), paintRect.bottomRight());
            painter.drawLine(paintRect.topRight(), paintRect.bottomLeft());

        }
    }

    if (_zoomEnabled && !_zoomLabel->isHidden()) {
        hideZoomLabel();
    }

    //int t = time.elapsed(); if (t > 1) {kDebug()<<"Paint: "<<time.elapsed()<<endl;}
}

void PageDisplay::adjustLayout()
{
    Q_ASSERT(_displaying);

    // Reset the display size
    if (_zoomEnabled) {
        _boundingSize = _displaySize = size() * ZOOM_MODES[_zoomIndex];
    } else {
        _boundingSize = _displaySize = size();
    }

    if (_singlePage) {
        QSize imageSize = _pixmap[0].size();

        Q_ASSERT(imageSize.isValid());

        // Scale the page so it fits snug in the widget
        imageSize.scale(_displaySize, Qt::KeepAspectRatio);
        _destRect[0].setSize(imageSize);

        // If we're zooming, try to shrink the display "window" to the image
        if (_zoomEnabled) {
            if (imageSize.height() > height()) {
                _displaySize.setHeight(imageSize.height());
            } else {
                _displaySize.setHeight(height());
            }
            if (imageSize.width() > width()) {
                _displaySize.setWidth(imageSize.width());
            } else {
                _displaySize.setWidth(width());
            }
        }

        // Nice things happen when when the image is prescaled
        _prescaled[0] = imageSize == _pixmap[0].size();

        // Check which dimension has extra space and adjust the
        //  weights of the image and spacers accordingly
        if (imageSize.height() == _displaySize.height()) {
            _destRect[0].moveTo((_displaySize.width() - imageSize.width()) / 2, 0);
        } else {
            _destRect[0].moveTo(0, (_displaySize.height() - imageSize.height()) / 2);
        }

        // The previous scaled pixmap isn't any use
        _scaledPixmap[0] = QPixmap();
        _doFullScale[0] = false;

    } else {
        Q_ASSERT(!_pixmap[1].isNull());

        QSize imageSize[2] = {_pixmap[0].size(), _pixmap[1].size()};

        //kDebug()<<"Image sizes "<<imageSize[0]<<imageSize[1]<<endl;

        Q_ASSERT(imageSize[0].isValid());
        Q_ASSERT(imageSize[1].isValid());

        // Bring both pages to the same height
        if (imageSize[0].height() > imageSize[1].height()) {
            imageSize[1].scale(0x7FFFFFFF, imageSize[0].height(), Qt::KeepAspectRatio);
        } else if (imageSize[1].height() > imageSize[0].height()) {
            imageSize[0].scale(0x7FFFFFFF, imageSize[1].height(), Qt::KeepAspectRatio);
        }
        Q_ASSERT(imageSize[0].height() == imageSize[1].height());

        // Calculate the total size
        QSize togetherSize(imageSize[0].width() + imageSize[1].width(), imageSize[0].height());

        // Scale the pages so they fit snug in the widget
        togetherSize.scale(_displaySize, Qt::KeepAspectRatio);

        _destRect[0].setWidth(int(
            qreal(togetherSize.width()) * (qreal(imageSize[0].width()) / qreal(imageSize[0].width() + imageSize[1].width()))
        ));
        _destRect[0].setHeight(togetherSize.height());
        _destRect[1].setWidth(togetherSize.width() - _destRect[0].width());
        _destRect[1].setHeight(togetherSize.height());

        // If we're zooming, try to shrink the display "window" to the image
        if (_zoomEnabled) {
            if (togetherSize.height() > height()) {
                _displaySize.setHeight(togetherSize.height());
            } else {
                _displaySize.setHeight(height());
            }
            if (togetherSize.width() > width()) {
                _displaySize.setWidth(togetherSize.width());
            } else {
                _displaySize.setWidth(width());
            }
        }

        // Nice things happen when when the image is prescaled
        _prescaled[0] = _destRect[0].size() == _pixmap[0].size();
        _prescaled[1] = _destRect[1].size() == _pixmap[1].size();

        // Check which dimension has extra space and adjust the
        //  weights of the image and spacers accordingly
        if (togetherSize.height() == _displaySize.height()) {
            _destRect[0].moveTo((_displaySize.width() - togetherSize.width()) / 2, 0);
            _destRect[1].moveTo(_destRect[0].right() + 1, 0);
        } else {
            _destRect[0].moveTo(0, (_displaySize.height() - togetherSize.height()) / 2);
            _destRect[1].moveTo(_destRect[0].right() + 1, _destRect[0].top());
        }

        //kDebug()<<"Widget "<<size()<<"display "<<_displaySize<<", dest rects "<<_destRect[0]<<_destRect[1]<<endl;

        // The previous scaled pixmaps aren't any use
        _scaledPixmap[0] = QPixmap();
        _scaledPixmap[1] = QPixmap();
        _doFullScale[0] = false;
        _doFullScale[1] = false;
    }

    if (_zoomEnabled) {
        // Determine where the viewport starts and how far it can move
        QRect bounds;
        QPoint offset;
        if (_displaySize.width() == width()) {
            // The pages are padded on the left and right
            bounds.setLeft(_destRect[0].left());
            bounds.setRight(_destRect[0].left());
            offset.setX(_destRect[0].left());

        } else {
            // The pages are free to move horizontally
            bounds.setLeft(width() - _displaySize.width());
            bounds.setRight(0);
            if (_mangaMode) {
                // Show the far right side of the images
                offset.setX(width() - _displaySize.width());
            } else {
                offset.setX(0);
            }
        }

        if (_displaySize.height() == height()) {
            // The pages are padded on the top and bottom
            bounds.setTop(_destRect[0].top());
            bounds.setBottom(_destRect[0].top());
            offset.setY(_destRect[0].top());

        } else {
            // The pages are free to move vertically
            bounds.setTop(height() - _displaySize.height());
            bounds.setBottom(0);
            offset.setY(0);
        }

        // Move the dest rects based on the current offset
        _destRect[0].moveTo(offset);
        if (!_singlePage) {
            Q_ASSERT(_destRect[0].height() == _destRect[1].height());
            _destRect[1].moveTo(offset + QPoint(_destRect[0].width(), 0));
        }

        _newMousePath = true;

        // Tell the display scroller to reset
        _displayScroller->reset(bounds, ZOOM_MODES[_zoomIndex], offset);
    }
}

void PageDisplay::resizeEvent(QResizeEvent *)
{
    kDebug()<<"Resized to "<<size()<<endl;

    // Don't move the labels, just hide them
    if (_pageNumberLabel->isVisible()) {
        _pageNumberTimer.stop();

        _pageNumberLabel->hide();
    }
    if (_zoomLabel->isVisible()) {
        _zoomTimer.stop();

        _zoomLabel->hide();
    }

    // If there are pages being displayed, position them correctly
    if (_displaying) {

        // Put the pages in the right places
        adjustLayout();

        // Tell the loader that the display size is changing
        emit displaySizeChanged(_boundingSize);

        update();
    } else {
        // Tell the loader that the display size is changing
        emit displaySizeChanged(size());
    }
}
void PageDisplay::enterEvent(QEvent *)
{
    //kDebug()<<"Mouse entered widget"<<endl;
    _newMousePath = true;
    _mouseOver = true;
    setCursor(_cursor);
}
void PageDisplay::leaveEvent(QEvent *)
{
    _mouseOver = false;
    setCursor(Qt::ArrowCursor);
}

void PageDisplay::mouseMoveEvent(QMouseEvent *event)
{
    //setMouseTracking(false);
    if (!_zoomEnabled) {
        event->ignore();

    } else if (_newMousePath) {
        _newMousePath = false;
        _previousMousePosition = event->pos();
        //kDebug()<<"Mouse started path "<<event->pos()<<endl;

        event->accept();

    } else {
        QPoint delta = event->pos() - _previousMousePosition;
        //kDebug()<<"Mouse moved "<<delta<<endl;

        _displayScroller->mouseMoved(-delta);

        _previousMousePosition = event->pos();

        event->accept();
    }

    // Show the mouse and start counting idle time
    mouseMoved();
    //setMouseTracking(true);
}

void PageDisplay::moveDisplay(const QPoint &newPosition)
{
    // Move the top-left corner of the display
    _destRect[0].moveTo(newPosition);
    if (!_singlePage) {
        Q_ASSERT(_destRect[0].height() == _destRect[1].height());
        _destRect[1].moveTo(newPosition + QPoint(_destRect[0].width(), 0));
    }

    //int previousUpdateStatus = updatesEnabled();
    //setUpdatesEnabled(false);

    repaint();
    //update();

    //setUpdatesEnabled(previousUpdateStatus);
}

void PageDisplay::mouseMoved()
{
    // Show the cursor and start counting idle time (if it's not zoom mode)
    if (_displaying && !_zoomEnabled) {
        changeCursor(Qt::ArrowCursor);

        const int MOUSE_HIDE_DELAY = 2500;
        _mouseIdleTimer.stop();
        _mouseIdleTimer.start(MOUSE_HIDE_DELAY, this);
    }
}

void PageDisplay::changeCursor(QCursor change)
{
    _cursor = change;
    if (_mouseOver) {
        setCursor(_cursor);
    }
}

void PageDisplay::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(_displaying);
    if (event->timerId() == _pageNumberTimer.timerId()) {
        // Hide the page number
        _pageNumberTimer.stop();
        _pageNumberLabel->hide();

    } else if (event->timerId() == _zoomTimer.timerId()) {
        // Hide the zoom label
        _zoomTimer.stop();
        _zoomLabel->hide();

    } else if (event->timerId() == _mouseIdleTimer.timerId()) {
        // Hide the cursor if pages are displayed (and it's not zoom mode)
        _mouseIdleTimer.stop();
        if (!_zoomEnabled) {
            changeCursor(Qt::BlankCursor);
        }

    } else {
        Q_ASSERT(false);
    }
}

#include "pagedisplay.moc"
