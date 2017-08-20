#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFileDialog>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTime>

#include "debug.h"
#include "steward.h"
#include "toolbarwidget.h"
#include "viewwidget.h"

MainWindow::MainWindow(const QString &initialArg, QWidget *parent)
    : QMainWindow(parent)
{
    // Make the steward
    _steward = new Steward(this);

    QWidget *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Add the toolbar
    ToolbarWidget *toolbar = new ToolbarWidget(*_steward, this);
    toolbar->hide();
    layout->addWidget(toolbar);

    // Add the view widget
    ViewWidget *view = new ViewWidget(*_steward, toolbar, this);
    layout->addWidget(view);

    // Enable things
    _zoomToggleEnabled = true;
    _zoomInEnabled = true;
    _zoomOutEnabled = true;

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

    // Open shortcut
    QAction *open = new QAction(this);
    open->setShortcut(Qt::Key_O);
    open->setShortcutContext(Qt::ApplicationShortcut);
    addAction(open);
    connect(open, SIGNAL(triggered()), SLOT(open()));

    // Fullscreen shortcut
    QAction *fullscreen = new QAction(this);
    fullscreen->setCheckable(true);
    fullscreen->setShortcut(Qt::Key_F);
    fullscreen->setShortcutContext(Qt::ApplicationShortcut);
    addAction(fullscreen);
    connect(fullscreen, SIGNAL(toggled(bool)), SLOT(fullscreen(bool)));

    // Quit shortcut
    QAction *quit = new QAction(this);
    quit->setShortcut(Qt::Key_Q);
    quit->setShortcutContext(Qt::ApplicationShortcut);
    addAction(quit);
    connect(quit, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));

    // Next page shortcut
    QAction *nextPage = new QAction(this);
    nextPage->setShortcuts(QList<QKeySequence>()<<Qt::Key_Space<<Qt::Key_PageDown<<Qt::Key_Left);
    nextPage->setShortcutContext(Qt::ApplicationShortcut);
    addAction(nextPage);
    connect(nextPage, SIGNAL(triggered()), SIGNAL(nextPage()));

    // Previous page shortcut
    QAction *previousPage = new QAction(this);
    previousPage->setShortcuts(QList<QKeySequence>()<<Qt::Key_PageUp<<Qt::Key_Right);
    previousPage->setShortcutContext(Qt::ApplicationShortcut);
    addAction(previousPage);
    connect(previousPage, SIGNAL(triggered()), SIGNAL(previousPage()));

    // Shift next page shortcut
    QAction *shiftNextPage = new QAction(this);
    shiftNextPage->setShortcut(Qt::Key_Tab);
    shiftNextPage->setShortcutContext(Qt::ApplicationShortcut);
    addAction(shiftNextPage);
    connect(shiftNextPage, SIGNAL(triggered()), SIGNAL(shiftNextPage()));
}

MainWindow::~MainWindow()
{
    debug()<<"~MainWindow()";
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 600); }

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
        debug()<<"Shifting";
        emit shiftNextPage();
    }

    // Right mouse toggles zoom
    if (event->button() == Qt::RightButton && _zoomToggleEnabled)
    {
    }
}

/**
 * @todo Get native KDE dialogs working like QtDesigner
 * @todo Open at current file
 * @todo Allow "no file", meaning the directory
 */
void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Book");

    if (!filename.isEmpty())
    {
        setSource(filename);
    }
}

void MainWindow::fullscreen(bool toggled)
{
    if (toggled)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }
}
