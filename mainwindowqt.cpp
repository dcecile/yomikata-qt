#include "mainwindowqt.h"

#include <QAction>
#include <QFileDialog>
#include <QApplication>

MainWindowQt::MainWindowQt(const QString &initialArg, QWidget *parent)
    : MainWindow(initialArg, parent)
{
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

MainWindowQt::~MainWindowQt()
{
}

/**
 * @todo Get native KDE dialogs working like QtDesigner
 * @todo Open at current file
 * @todo Allow "no file", meaning the directory
 */
void MainWindowQt::open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Book");

    if (!filename.isEmpty())
    {
        setSource(filename);
    }
}

void MainWindowQt::fullscreen(bool toggled)
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

#include "mainwindowqt.moc"
