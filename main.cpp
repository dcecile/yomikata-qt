#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

#include "yomikata.h"

int main (int argc, char *argv[])
{
    KAboutData aboutData("yomikata",
                         "yomikata", "0.0", I18N_NOOP("A straight-forward app for reading comics and manga."),
                        KAboutData::License_GPL, "(c) 2006");
    aboutData.addAuthor("Dan Cecile", 0, "dancecile@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    Yomikata *window = new Yomikata();
    window->show();

    return app.exec();
}
