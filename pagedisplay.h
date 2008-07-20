#ifndef PAGEDISPLAY_H
#define PAGEDISPLAY_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QBasicTimer>

class DisplayScroller;

class PageDisplay : public QWidget
{
    Q_OBJECT

public:
    PageDisplay(QWidget *parent = 0);

signals:
    void displaySizeChanged(const QSize &newSize);

    void zoomToggleEnabled(bool enabled);
    void zoomInEnabled(bool enabled);
    void zoomOutEnabled(bool enabled);

public slots:
    void loadingStarted();

    void setMangaMode(bool enabled);

    void setOnePage(const QPixmap &pixmap, int pageNum, int totalPages);
    void setTwoPages(const QPixmap &pixmapA, const QPixmap &pixmapB, int pageNumA, int pageNumB, int totalPages);

    void setPageNumber(int pageNumA, int pageNumB, int totalPages);

    void toggleZoom();
    void zoomIn();
    void zoomOut();

private:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void adjustLayout();

    void resizeEvent(QResizeEvent *event);

    void timerEvent(QTimerEvent *event);

    void mouseMoved();

    void showZoomLabel();
    void hideZoomLabel();

    void updateZoomStatus();

    void changeCursor(QCursor change);

private slots:
    void moveDisplay(const QPoint &newPosition);

private:
    bool _displaying;

    bool _mangaMode;

    bool _singlePage;

    QPixmap _pixmap[2];
    QPixmap _scaledPixmap[2];
    bool _doFullScale[2];
    QRect _destRect[2];
    bool _prescaled[2];

    QLabel *_pageNumberLabel;
    QBasicTimer _pageNumberTimer;

    QSize _displaySize;
    QSize _boundingSize;

    bool _zoomEnabled;
    int _zoomIndex;
    bool _newMousePath;
    QPoint _previousMousePosition;

    QLabel *_zoomLabel;
    QBasicTimer _zoomTimer;

    DisplayScroller *_displayScroller;

    QBasicTimer _mouseIdleTimer;

    bool _mouseOver;
    QCursor _cursor;

private:
    static const qreal ZOOM_MODES[];
};

#endif
