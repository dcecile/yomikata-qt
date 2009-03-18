#include "mainwindow.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTime>

#include "debug.h"
#include "steward.h"
#include "toolbarwidget.h"
#include "viewwidget.h"

MainWindow::MainWindow(const QString &initialArg, QWidget *parent)
    : MainWindowAncestor(parent)
{
    // Make the steward
    _steward = new Steward(this);

    QWidget *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Add the toolbar
    ToolbarWidget *toolbar = new ToolbarWidget(this);
    toolbar->hide();
    layout->addWidget(toolbar);

    // Add the view widget
    ViewWidget *view = new ViewWidget(*_steward, toolbar, this);
    layout->addWidget(view);

    // Enable things
    _zoomToggleEnabled = true;
    _zoomInEnabled = true;
    _zoomOutEnabled = true;

    // Connect to toolbar
    connect(toolbar, SIGNAL(open()), SLOT(open()));
    connect(toolbar, SIGNAL(quit()), SLOT(close()));
    connect(toolbar, SIGNAL(nextPage()), SIGNAL(nextPage()));
    connect(toolbar, SIGNAL(previousPage()), SIGNAL(previousPage()));
    connect(toolbar, SIGNAL(shiftNextPage()), SIGNAL(shiftNextPage()));
    connect(toolbar, SIGNAL(fullscreen(bool)), SLOT(fullscreen(bool)));

    // Connect to controls
    connect(this, SIGNAL(nextPage()), _steward, SLOT(next()));
    connect(this, SIGNAL(previousPage()), _steward, SLOT(previous()));
    connect(this, SIGNAL(shiftNextPage()), _steward, SLOT(shiftNext()));

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
        QTime clock;
        clock.start();

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

        debug()<<"Turning"<<clock.elapsed()<<"ms";
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
