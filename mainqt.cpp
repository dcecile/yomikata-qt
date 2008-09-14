#include <QApplication>

#include "mainwindowqt.h"

#ifndef UNIT_TESTING
int main(int argc, char *argv[])
#else
int application_main(int argc, char *argv[])
#endif
{
    QApplication app(argc, argv);

    MainWindow *window = new MainWindowQt("");
    window->show();

    return app.exec();
}
