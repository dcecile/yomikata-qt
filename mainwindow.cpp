#include "mainwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QSplitter>

#include "debug.h"
#include "steward.h"

MainWindow::MainWindow(const QString &initialArg, QWidget *parent)
    : MainWindowAncestor(parent)
{
    // Make the steward
    _steward = new Steward(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(splitter);

    // Add the projector
    QWidget *projector = _steward->projector();
    projector->setParent(this);
    splitter->addWidget(projector);

    // Add the debug widget
    QWidget *debugWidget = _steward->debugWidget();
    debugWidget->setParent(this);
    splitter->addWidget(debugWidget);

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
    _zoomToggleEnabled = true;
    _zoomInEnabled = true;
    _zoomOutEnabled = true;
}

MainWindow::~MainWindow()
{
    debug()<<"~MainWindow()";
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 600);
}

void MainWindow::setSource(const QString &filename)
{
    debug()<<"Opening file"<<filename;
    _steward->reset(filename);
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
        if (event->delta() > 0)
        {
            // Page back
            _steward->previous();
        }
        else if (event->delta() < 0)
        {
            // Page forward
            _steward->next();
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
    if (event->button() == Qt::MidButton)
    {
        _steward->shiftNext();
    }

    // Right mouse toggles zoom
    if (event->button() == Qt::RightButton && _zoomToggleEnabled)
    {
    }
}

#include "mainwindow.moc"
