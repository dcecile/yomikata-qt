#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QFrame>
#include <QTimeLine>

class QPushButton;

class ToolbarWidget : public QFrame
{
    Q_OBJECT

public:
    ToolbarWidget(QWidget *parent = NULL);
    ~ToolbarWidget();

    void startShow();
    void startHide();

signals:
    void nextPage();
    void previousPage();
    void shiftNextPage();
    void fullscreen(bool toggled);
    void open();
    void quit();

private:
    QPushButton *makeButton(const QString &text);

private slots:
    void setShownHeight(int height);

private:
    static const int FRAME_WIDTH;
    static const int SLIDE_DURATION;
    static const float SLIDE_FRAMES_PER_SECOND;
    int _fullHeight;
    QTimeLine _animation;
    bool _isShowing;
};

#endif
