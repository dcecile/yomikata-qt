#include <QApplication>
#include <QIcon>
#include <QSettings>

#include <QProcess>
#include <QFile>
#include <QTextCodec>

#include "mainwindowqt.h"
#include "debug.h"

#ifndef UNIT_TESTING
int main(int argc, char *argv[])
#else
int application_main(int argc, char *argv[])
#endif
{
    QApplication app(argc, argv);

#ifndef Q_OS_WIN32
    // Icon not needed on windows
    app.setWindowIcon(QIcon(":/icon.ico"));
#endif

    // Set settings location
    QCoreApplication::setOrganizationName("yomikata");
    QCoreApplication::setApplicationName("yomikata");

#ifdef Q_OS_WIN32
    // Don't use the Windows registry
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

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
