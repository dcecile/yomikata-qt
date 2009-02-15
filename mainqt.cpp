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
    app.setWindowIcon(QIcon(":/icon_16.png"));

    /*
    QFile in("examples/list.txt");
    in.open(QIODevice::ReadOnly);
    QByteArray data = in.readLine();

    QString text = QTextCodec::codecForLocale()->toUnicode(data);

    QFile utf16("examples/utf16.txt");
    utf16.open(QIODevice::WriteOnly);
    utf16.write((const char *) text.utf16(), text.length() * 2);
    utf16.close();

    QFile utf8("examples/utf8.txt");
    utf8.open(QIODevice::WriteOnly);
    utf8.write(text.toUtf8());
    utf8.close();

    QFile latin1("examples/latin1.txt");
    latin1.open(QIODevice::WriteOnly);
    latin1.write(text.toLatin1());
    latin1.close();

    QFile system("examples/system.txt");
    system.open(QIODevice::WriteOnly);
    system.write(QTextCodec::codecForLocale()->fromUnicode(text));
    system.close();

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QProcess test;
    test.start("cmd", QStringList()<<"/C"<<"echo"<<text);
    test.waitForStarted();
    test.waitForFinished();

    QFile out("examples/out.txt");
    out.open(QIODevice::WriteOnly);
    out.write(test.readAll());
    out.close();

    QFile systemReset("examples/systemReset.txt");
    systemReset.open(QIODevice::WriteOnly);
    systemReset.write(QTextCodec::codecForLocale()->fromUnicode(text));
    systemReset.close();

    Q_ASSERT(false);
    //*/

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
