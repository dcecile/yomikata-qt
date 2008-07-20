#ifndef PAGEDISPLAY_H
#define PAGEDISPLAY_H

#include <QWidget>
#include <QPixmap>

class PageDisplay : public QWidget
{
    Q_OBJECT

public:
    PageDisplay(QWidget *parent = 0);

    void setOnePage(const QPixmap &pixmap);
    void setTwoPages(const QPixmap &pixmapA, const QPixmap &pixmapB);

private:
    void paintEvent(QPaintEvent *event);

    void adjustLayout();

    void resizeEvent(QResizeEvent *event);

private:
    bool _singlePage;

    QPixmap _pixmap[2];
    QPixmap _scaledPixmap[2];
    QRect _destRect[2];
    bool _prescaled[2];
};

#endif
