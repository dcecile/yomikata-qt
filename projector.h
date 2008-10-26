#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QWidget>

class QLabel;

class Projector : public QWidget
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

    int heightForWidth(int width);

signals:
    void resized(const QSize &size);

private:
    QWidget *createLoadingWidget();

    void resizeEvent(QResizeEvent *event);

private:
    QWidget *_loading0;
    QWidget *_loading1;
    QLabel *_page0;
    QLabel *_page1;
};

#endif
