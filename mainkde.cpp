#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

#include "mainwindowkde.h"
#include "debug.h"

#ifndef UNIT_TESTING
int main (int argc, char *argv[])
#else
int application_main (int argc, char *argv[])
#endif
{
    KAboutData aboutData("yomikata",
            "",
            ki18n("yomikata"),
            "0.0",
            ki18n("A straight-forward app for reading comics and manga."),
            KAboutData::License_GPL,
            ki18n("(c) 2007-2008"));
    aboutData.addAuthor(ki18n("Dan Cecile"), ki18n("Programming"), "dancecile@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[file | directory]", ki18n("The file or directory to start at"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString initialArg;

    if (args->count() == 1)
    {
        initialArg = args->arg(0);
        debug()<<"Passed file:"<<initialArg;

    }
    else if (args->count() > 1)
    {
        args->usageError(i18n("Only one initial file or directory is supported."));
    }

    MainWindow *window = new MainWindowQt(initialArg);
    window->show();

    return app.exec();
}
