#include "mainwindow.h"

#include <QLabel>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>

#include "debug.h"

MainWindow::MainWindow(const QString &initialArg, QWidget *parent)
    : MainWindowAncestor(parent)
{
    setCentralWidget(new QLabel("Hello world", this));

    // Start opening the initial file
    if (!initialArg.isEmpty())
    {
        // Open the file right away
        if (QDir::isRelativePath(initialArg))
        {
            setSource(QDir::current().absoluteFilePath(initialArg));
        }
        else
        {
            setSource(initialArg);
        }
    }

    // Enable things
    _pageForwardEnabled = true;
    _pageBackwardEnabled = true;
    _zoomToggleEnabled = true;
    _zoomInEnabled = true;
    _zoomOutEnabled = true;
}

MainWindow::~MainWindow()
{
}

QSize MainWindow::sizeHint() const
{
    return QSize(300, 200);
}

void MainWindow::setSource(const QString &initialFile)
{
    debug()<<"Opening file"<<initialFile;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    // Note: allows scrolling on menubar, similar to most other KDE apps

    // See if CTRL or SHIFT is being pressed
    bool zoom = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) != 0;

    if (zoom)
    {
        if (event->delta() > 0 && _zoomInEnabled)
        {
            // Zoom in

        } else if (event->delta() < 0 && _zoomOutEnabled)
        {
            // Zoom out

        } else {
            // Can't zoom
            event->ignore();
        }

    }
    else
    {
        if (event->delta() > 0 && _pageBackwardEnabled)
        {
            // Page back
            debug()<<"Previous";

        }
        else if (event->delta() < 0 && _pageForwardEnabled)
        {
            // Page forward
            debug()<<"Next";

        }
        else
        {
            // Can't page
            event->ignore();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // Middle mouse triggers paging forward one page
    if (event->button() == Qt::MidButton && _pageForwardEnabled)
    {
    }

    // Right mouse toggles zoom
    if (event->button() == Qt::RightButton && _zoomToggleEnabled)
    {
    }
}

#include "mainwindow.moc"
