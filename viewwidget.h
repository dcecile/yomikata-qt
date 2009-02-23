#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class Steward;

class ViewWidget : public QWidget
{
    Q_OBJECT

public:
    ViewWidget(Steward &steward, QWidget *parent = NULL);
    ~ViewWidget();

    QSize sizeHint() const;
    int heightForWidth(int width) const;

private:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Steward &_steward;
};

#endif
