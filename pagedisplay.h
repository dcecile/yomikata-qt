#ifndef PAGEDISPLAY_H
#define PAGEDISPLAY_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QBasicTimer>

class PageDisplay : public QWidget
{
    Q_OBJECT

public:
    PageDisplay(QWidget *parent = 0);

signals:
    void displaySizeChanged(const QSize &newSize);

public slots:
    void setMangaMode(bool enabled);

    void setOnePage(const QPixmap &pixmap, int pageNum, int totalPages);
    void setTwoPages(const QPixmap &pixmapA, const QPixmap &pixmapB, int pageNumA, int pageNumB, int totalPages);

    void setPageNumber(int pageNumA, int pageNumB, int totalPages);

private:
    void paintEvent(QPaintEvent *event);

    void adjustLayout();

    void resizeEvent(QResizeEvent *event);

    void timerEvent(QTimerEvent *event);

private:
    bool _mangaMode;

    bool _singlePage;

    QPixmap _pixmap[2];
    QPixmap _scaledPixmap[2];
    QRect _destRect[2];
    bool _prescaled[2];

    QLabel *_pageNumberLabel;
    QBasicTimer _pageNumberTimer;
};

#endif
