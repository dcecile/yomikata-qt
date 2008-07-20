#ifndef PRESENTER_H
#define PRESENTER_H

#include <QWidget>
#include <QTime>

#include "lens.h"

/**
 * @defgroup mod_presenter Presenter module
 * Display widget and display controls
 */

/**
 * @ingroup mod_presenter
 * @brief The presenter widget
 */
class Presenter : public QWidget
{
    Q_OBJECT

public:
    Presenter(Lens &lensA, Lens &lensB, QWidget *parent);

    ~Presenter();

signals:
    void displayAreaChanged(const QSize &newSize);

private:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Lens &_lensA;
    Lens &_lensB;

    bool _mangaMode;

    QTime _resizeTime;
    bool _resizePainted;
};

#endif
