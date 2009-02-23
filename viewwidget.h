#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QFrame>

class Steward;

class ViewWidget : public QWidget
{
    Q_OBJECT

public:
    ViewWidget(Steward &steward, QWidget *parent = NULL);
    ~ViewWidget();

    QSize sizeHint() const;
    int heightForWidth(int width) const;

signals:
    void showToolbar();
    void hideToolbar();

private:
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Steward &_steward;
    bool _usingToolbar;
};

#endif
