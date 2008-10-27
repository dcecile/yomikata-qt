#include <QApplication>
#include <QIcon>

#include "mainwindowqt.h"

#ifndef UNIT_TESTING
int main(int argc, char *argv[])
#else
int application_main(int argc, char *argv[])
#endif
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon_16.png"));

    // Initial argument
    QString arg = "";

    if (argc >= 2)
    {
        arg = argv[1];
    }

    // Qt doesn't destory main windows automatically
    MainWindowQt window(arg);
    window.show();

    return app.exec();
}