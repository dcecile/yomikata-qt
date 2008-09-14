#include <QApplication>

#include "mainwindowqt.h"

#ifndef UNIT_TESTING
int main(int argc, char *argv[])
#else
int application_main(int argc, char *argv[])
#endif
{
    QApplication app(argc, argv);

    // Qt doesn't destory main windows automatically
    MainWindowQt window("");
    window.show();

    return app.exec();
}
