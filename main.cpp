#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <QTextCodec>

#include "yomikata.h"

static const KCmdLineOptions options[] =
{
    { "+[file | directory]", I18N_NOOP("The file or directory to start at"), 0 },
    KCmdLineLastOption // End of options.
};

int main (int argc, char *argv[])
{
    KAboutData aboutData("yomikata",
                         "yomikata", "0.0", I18N_NOOP("A straight-forward app for reading comics and manga."),
                        KAboutData::License_GPL, "(c) 2006");
    aboutData.addAuthor("Dan Cecile", 0, "dancecile@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString initialArg;
    if (args->count() == 1) {
        initialArg = QTextCodec::codecForName("utf-8")->toUnicode(args->arg(0));
        kDebug()<<"Passed file: "<<QString()<<endl;

    } else if (args->count() > 1) {
        args->usage(i18n("Only one initial file or directory is supported."));
    }

    Yomikata *window = new Yomikata(initialArg);
    window->show();

    return app.exec();
}
