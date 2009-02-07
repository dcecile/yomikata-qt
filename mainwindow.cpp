#include "mainwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QSplitter>

#include "debug.h"
#include "steward.h"
#include "loadingwidget.h"

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
    _debugWidget = _steward->debugWidget();
    _debugWidget->setParent(this);
    splitter->addWidget(_debugWidget);
    _debugWidget->hide();

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

    // Connect
    connect(this, SIGNAL(nextPage()), _steward, SLOT(next()));
    connect(this, SIGNAL(previousPage()), _steward, SLOT(previous()));
    connect(this, SIGNAL(shiftNextPage()), _steward, SLOT(shiftNext()));
}

MainWindow::~MainWindow()
{
    debug()<<"~MainWindow()";
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 600);
}

void MainWindow::showDebugWidget(bool toggled)
{
    if (toggled)
    {
        _debugWidget->show();
    }
    else
    {
        _debugWidget->hide();
    }
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
            for (int i = 0; i < event->delta() / 120; i++)
            {
                emit previousPage();
            }
        }
        else if (event->delta() < 0)
        {
            // Page forward
            for (int i = 0; i < event->delta() / -120; i++)
            {
                emit nextPage();
            }
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
        emit shiftNextPage();
    }

    // Right mouse toggles zoom
    if (event->button() == Qt::RightButton && _zoomToggleEnabled)
    {
    }
}

#include "mainwindow.moc"
