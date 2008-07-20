#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <KDebug>
#include <QTextCodec>
#include <QByteArray>
#include <QString>

#include "yomikata.h"

/**
 * @mainpage
 * Yomikata: A liberated and enjoyable comics reading experience.
 *
 * @section cast Cast
 *
 * There are eight main actors in this performance:
 * - Yomikata: sets up the GUI and recieves input events
 * - Oyabun: responds to input events, and uses the Book, the Decoder and the Depicter to get images from the disk to the screen
 * - Book: filled with @ref Page "Pages", knows where it's opened to
 * - Decoder: decodes @ref Page "Pages"
 * - Depicter: displays @ref Page "Pages" and depicts the loading progress of @ref Page "Pages", controls zooming and panning
 * - Page: all specific info: decoded image, planned dimensions, and loading status (uses Planner and Cache underneath)
 * - Planner: knows, display area size and the full dimensions of each page, plans where pages are put
 * - Cache: keeps all decoded images, decaches cold pages when cache full
 *
 * @section plot Plot
 *
 * The program starts in @ref main and opens up the main window Yomikata. Yomikata sets up the UI, then displays itself.
 *
 * As UI events are triggered, the first come to Yomikata. Anything reading related gets forwarded to Oyabun. Oyabun will open the book, turn pages in it, request decodes to the Decoder and notify the Depicter of pages changing.
 */

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
        kDebug()<<"Passed file:"<<initialArg;

    } else if (args->count() > 1) {
        args->usageError(i18n("Only one initial file or directory is supported."));
    }

    Yomikata *window = new Yomikata(initialArg);
    window->show();

    return app.exec();
}
