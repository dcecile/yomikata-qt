#include "mainwindow.h"

#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>

MainWindow::MainWindow(QWidget *parent) : KMainWindow(parent)
{
  textArea = new KTextEdit;
  setCentralWidget(textArea);

  setupActions();
}

void MainWindow::setupActions()
{
  KAction* clearAction = new KAction(actionCollection(), "clear");
  //QAction *clearAction = actionCollection()->addAction("clear");
  clearAction->setText(i18n("Clear"));
  clearAction->setIcon(KIcon("filenew"));
  clearAction->setShortcut(Qt::CTRL+Qt::Key_W);
  connect(clearAction, SIGNAL(triggered(bool)),
          textArea, SLOT(clear()));

  KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

  setupGUI();
}
