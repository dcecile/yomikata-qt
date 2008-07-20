#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KMainWindow>
#include <KTextEdit>

class MainWindow : public KMainWindow
{
public:
  MainWindow(QWidget *parent=0);

private:
  KTextEdit *textArea;
  void setupActions();
};

#endif
