#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QWidget>

class ToolbarWidget : public QWidget
{
    Q_OBJECT

public:
    ToolbarWidget(QWidget *parent = NULL);
    ~ToolbarWidget();

signals:
    void open();
    void quit();
};

#endif
