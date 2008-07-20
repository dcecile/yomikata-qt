#include <QPainter>
#include <QPaintEvent>
#include <KDebug>

#include "pagedisplay.h"

PageDisplay::PageDisplay(QWidget *parent)
    :QWidget(parent)
{
    _mangaMode = true;

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
            if (paintRect == _destRect[i]) {
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
            if (paintRect == _destRect[i]) {
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

    QSize totalSize = size();

    if (_singlePage) {
        QSize imageSize = _pixmap[0].size();

        Q_ASSERT(imageSize.isValid());

        // Scale the page so it fits snug in the widget
        imageSize.scale(totalSize, Qt::KeepAspectRatio);
        _destRect[0].setSize(imageSize);

        // Nice things happen when when the image is prescaled
        _prescaled[0] = imageSize == _pixmap[0].size();

        // Check which dimension has extra space and adjust the
        //  weights of the image and spacers accordingly
        if (imageSize.height() == totalSize.height()) {
            _destRect[0].moveTo((totalSize.width() - imageSize.width()) / 2, 0);
        } else {
            _destRect[0].moveTo(0, (totalSize.height() - imageSize.height()) / 2);
        }

        // The previous scaled pixmap isn't any use
        _scaledPixmap[0] = QPixmap();

    } else {
        Q_ASSERT(!_pixmap[1].isNull());

        QSize imageSize[2] = {_pixmap[0].size(), _pixmap[1].size()};

        kDebug()<<"Image sizes "<<imageSize[0]<<imageSize[1]<<endl;

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
        togetherSize.scale(totalSize, Qt::KeepAspectRatio);

        _destRect[0].setWidth(int(
            qreal(togetherSize.width()) * (qreal(imageSize[0].width()) / qreal(imageSize[0].width() + imageSize[1].width()))
        ));
        _destRect[0].setHeight(togetherSize.height());
        _destRect[1].setWidth(togetherSize.width() - _destRect[0].width());
        _destRect[1].setHeight(togetherSize.height());

        // Nice things happen when when the image is prescaled
        _prescaled[0] = _destRect[0].size() == _pixmap[0].size();
        _prescaled[1] = _destRect[1].size() == _pixmap[1].size();

        // Check which dimension has extra space and adjust the
        //  weights of the image and spacers accordingly
        if (togetherSize.height() == totalSize.height()) {
            _destRect[0].moveTo((totalSize.width() - togetherSize.width()) / 2, 0);
            _destRect[1].moveTo(_destRect[0].right() + 1, 0);
        } else {
            _destRect[0].moveTo(0, (totalSize.height() - togetherSize.height()) / 2);
            _destRect[1].moveTo(_destRect[0].right() + 1, _destRect[0].top());
        }

        kDebug()<<"Widget "<<size()<<", dest rects "<<_destRect[0]<<_destRect[1]<<endl;

        // The previous scaled pixmaps aren't any use
        _scaledPixmap[0] = QPixmap();
        _scaledPixmap[1] = QPixmap();
    }

    update();
}

void PageDisplay::resizeEvent(QResizeEvent *)
{
    // Tell the loader that the display size is changing
    emit displaySizeChanged(size());

    // Don't move the label, just hide it
    if (_pageNumberLabel->isVisible()) {
        _pageNumberTimer.stop();

        _pageNumberLabel->hide();
    }

    // If there are pages being displayed, position them correctly
    if (!_pixmap[0].isNull()) {

        // Put the pages in the right places
        adjustLayout();

        update();
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
