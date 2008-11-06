#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QScrollArea>

class QLabel;

class Projector : public QScrollArea
{
    Q_OBJECT

public:
    Projector(QWidget *parent = NULL);
    ~Projector();

    QSize sizeHint() const;

    void showBlank();

    void showLoading0(const QRect &rect);
    void showLoading1(const QRect &rect);
    void showPage0(const QRect &rect, QPixmap image);
    void showPage1(const QRect &rect, QPixmap image);

    void updatePosition0(const QRect &rect);
    void updatePosition1(const QRect &rect);

    int heightForWidth(int width) const;

public slots:
    void pagesChanged();

signals:
    void resized(const QSize &size);

private:
    QWidget *createLoadingWidget();
    void updateViewport();
    QRect displayArea();

    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    QRect _placement[2];
    QWidget *_target;
    QWidget *_loading0;
    QWidget *_loading1;
    QLabel *_page0;
    QLabel *_page1;
};

#endif
