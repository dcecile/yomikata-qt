#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <QTextCodec>
#include <QByteArray>
#include <QString>

#include "yomikata.h"

int main (int argc, char *argv[])
{
    KAboutData aboutData("yomikata",
                         "",
                         ki18n("yomikata"),
                         "0.0",
                         ki18n("A straight-forward app for reading comics and manga."),
                         KAboutData::License_GPL,
                         ki18n("(c) 2007"));
    aboutData.addAuthor(ki18n("Dan Cecile"), ki18n("Programming"), "dancecile@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[file | directory]", ki18n("The file or directory to start at"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString initialArg;
    if (args->count() == 1) {
        initialArg = args->arg(0);
        kDebug()<<"Passed file: "<<initialArg<<endl;

    } else if (args->count() > 1) {
        args->usageError(i18n("Only one initial file or directory is supported."));
    }

    Yomikata *window = new Yomikata(initialArg);
    window->show();

    return app.exec();
}
