#include "mainwindowqt.h"

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

MainWindowQt::MainWindowQt(const QString &initialArg, QWidget *parent)
    : MainWindow(initialArg, parent)
{
    // Open shortcut
    QAction *open = new QAction(this);
    open->setShortcut(Qt::Key_O);
    open->setShortcutContext(Qt::ApplicationShortcut);
    connect(open, SIGNAL(triggered()), SLOT(open()));

    // Make sure it's attached to a widget
    addAction(open);
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

#include "mainwindowqt.moc"
