#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QFrame>
#include <QImage>

class Steward;
class ToolbarWidget;

class ViewWidget : public QWidget
{
    Q_OBJECT

public:
    ViewWidget(Steward &steward, ToolbarWidget *toolbar, QWidget *parent = NULL);
    ~ViewWidget();

    QSize sizeHint() const;
    int heightForWidth(int width) const;

private:
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    int toolbarHeight() const;

private:
    static const int SHADOW_WIDTH;
    static const int SHADOW_HEIGHT;
    static const int SHADOW_TONE;
    static const int SHADOW_FADE;
    Steward &_steward;
    ToolbarWidget *_toolbar;
    bool _usingToolbar;
    QImage _shadow;
};

#endif
