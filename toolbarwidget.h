#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QFrame>
#include <QTimeLine>

class QPushButton;
class QLabel;
class QSlider;

class Steward;

class ToolbarWidget : public QFrame
{
    Q_OBJECT

public:
    ToolbarWidget(Steward &steward, QWidget *parent = NULL);
    ~ToolbarWidget();

    void startShow();
    void startHide();

private:
    QPushButton *makeButton(const QString &text);

private slots:
    void setShownHeight(int height);

private slots:
    void pageChanged(int page, int total);

private:
    static const int FRAME_WIDTH;
    static const int SLIDE_DURATION;
    static const float SLIDE_FRAMES_PER_SECOND;

    Steward &_steward;
    int _fullHeight;
    QTimeLine _animation;
    bool _isShowing;
    QLabel *_page;
    QSlider *_seek;
};

#endif
