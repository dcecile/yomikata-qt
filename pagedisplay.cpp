#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <KDebug>

#include "pagedisplay.h"

PageDisplay::PageDisplay(QWidget *parent)
    :QWidget(parent)
{
    _mangaMode = true;
    _zoomEnabled = false;
    _displaySize = size();

    _pageNumberLabel = new QLabel(this);
    _pageNumberLabel->hide();
    _pageNumberLabel->setBackgroundRole(QPalette::Window);
    _pageNumberLabel->setAutoFillBackground(true);
    _pageNumberLabel->setAlignment(Qt::AlignCenter);
}

void PageDisplay::setMangaMode(bool enabled)
{
    _mangaMode = enabled;
}

void PageDisplay::setOnePage(const QPixmap &pixmap, int pageNum, int totalPages)
{
    // Make sure no painting gets done while the pages are changing
    int previousUpdateStatus = updatesEnabled();
    setUpdatesEnabled(false);

    // There is a new pixmap to display
    _pixmap[0] = pixmap;
    _scaledPixmap[0] = QPixmap();
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
    // Don't toggle the zoom if nothing's being displayed
    if (_pixmap[0].isNull()) {
        return;
    }

    _zoomEnabled = !_zoomEnabled;
    setMouseTracking(_zoomEnabled);
    _newMousePath = true;

    if (_zoomEnabled) {
        setCursor(Qt::BlankCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

    adjustLayout();

    update();

    // Tell the loader that the display size is changing
    emit displaySizeChanged(_displaySize);
}

void PageDisplay::paintEvent(QPaintEvent *event)
{
    // Make sure we have something to paint
    if (_pixmap[0].isNull()) {
        return;
    }

    for (int i=0; i<(_singlePage?1:2); i++) {
        QRect paintRect(event->rect().intersected(_destRect[i]));

        if (paintRect.isEmpty()) {
            continue;
        }

        QPainter painter(this);

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
            if (_scaledPixmap[i].isNull()) {
                // Scale the pixmap up/down for the first time
                _scaledPixmap[i] = _pixmap[i].scaled(_destRect[i].size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
            painter.drawLine(_destRect[i].topLeft(), _destRect[i].bottomRight());
            painter.drawLine(_destRect[i].topRight(), _destRect[i].bottomLeft());
        }
    }
}

void PageDisplay::adjustLayout()
{
    Q_ASSERT(!_pixmap[0].isNull());

    // Reset the display size
    if (_zoomEnabled) {
        _displaySize = size() * 2;
    } else {
        _displaySize = size();
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
            }
            if (imageSize.width() > width()) {
                _displaySize.setWidth(imageSize.width());
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
            }
            if (togetherSize.width() > width()) {
                _displaySize.setWidth(togetherSize.width());
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

        //kDebug()<<"Widget "<<size()<<", dest rects "<<_destRect[0]<<_destRect[1]<<endl;

        // The previous scaled pixmaps aren't any use
        _scaledPixmap[0] = QPixmap();
        _scaledPixmap[1] = QPixmap();
    }

    if (_zoomEnabled) {
        // Initialize the viewport position
        if (_mangaMode) {
            // Show the far right side of the images
            _offset.setX(width() - _displaySize.width());
            _offset.setY(0);
        } else {
            _offset.setX(0);
            _offset.setY(0);
        }

        // Move the dest rects based on the current offset
        _destRect[0].translate(_offset);
        if (!_singlePage) {
            _destRect[1].translate(_offset);
        }

        _newMousePath = true;

        /*// Trim the dest rects to the size of the the widget
        _destRect[0] &= rect();
        if (!_singlePage) {
            _destRect[1] &= rect();
        }*/
    }
}

void PageDisplay::resizeEvent(QResizeEvent *)
{
    // Don't move the label, just hide it
    if (_pageNumberLabel->isVisible()) {
        _pageNumberTimer.stop();

        _pageNumberLabel->hide();
    }

    // If there are pages being displayed, position them correctly
    if (!_pixmap[0].isNull()) {

        // Put the pages in the right places
        adjustLayout();

        // Tell the loader that the display size is changing
        emit displaySizeChanged(size());

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
}

void PageDisplay::mouseMoveEvent (QMouseEvent * event)
{
    if (!_zoomEnabled) {
        event->ignore();

    } else if (_newMousePath) {
        _newMousePath = false;
        _previousMousePosition = event->pos();
        //kDebug()<<"Mouse started path "<<event->pos()<<endl;

    } else {
        QPoint delta = _previousMousePosition - event->pos();
        //kDebug()<<"Mouse moved "<<delta<<endl;

        delta *= 2;

        if (_offset.x() + delta.x() < width() - _displaySize.width()) {
            delta.setX(width() - _displaySize.width() - _offset.x());
        } else if (_offset.x() + delta.x() > 0) {
            delta.setX(-_offset.x());
        }
        if (_offset.y() + delta.y() < height() - _displaySize.height()) {
            delta.setY(height() - _displaySize.height() - _offset.y());
        } else if (_offset.y() + delta.y() > 0) {
            delta.setY(-_offset.y());
        }

        if (!delta.isNull()) {
            _offset += delta;
            //kDebug()<<"Delta capped at "<<delta<<endl;

            // Move the dest rects based on the current offset
            _destRect[0].translate(delta);
            if (!_singlePage) {
                _destRect[1].translate(delta);
            }

            update();
        }

        _previousMousePosition = event->pos();
    }
}

void PageDisplay::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == _pageNumberTimer.timerId());

    // Hide the page number
    _pageNumberTimer.stop();
    _pageNumberLabel->hide();
}

#include "pagedisplay.moc"
