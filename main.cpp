#include <QApplication>
#include <QIcon>
#include <QSettings>

#include <QProcess>
#include <QFile>
#include <QTextCodec>

#include "mainwindow.h"
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

    if (QCoreApplication::arguments().size() >= 2)
    {
        arg = QCoreApplication::arguments()[1];
    }

    // Qt doesn't destory main windows automatically
    MainWindow window(arg);
    window.show();

    return app.exec();
}
