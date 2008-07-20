
#include "yomikata.h"

#include <KApplication>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KFileDialog>
#include <KUrl>
#include <KMessageBox>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <KDebug>
#include <QWheelEvent>
#include <KMenuBar>
#include <KToolBar>
#include <KToggleFullScreenAction>
#include <QDir>
#include <KStatusBar>

#include "settings.h"
#include "fileclassifier.h"

Yomikata::Yomikata(const QString &initialArg, QWidget *parent)
    :KXmlGuiWindow(parent), _oyabun(this)
{
    PageMode pageMode = (PageMode)Settings::viewMode();
    Q_ASSERT(pageMode == SingleMode || pageMode == ComicsMode || pageMode == MangaMode);

    // Create the widget for displaying the pages
    setCentralWidget(_oyabun.getDepicter());

    // Set up the actions (open, quit, etc.)
    createActions();

    // Not much point in a small window
    setAppDefaults();//setMinimumSize(200, 200);

    // Load default toolbars, shortcuts
    setupGUI();
    menuBar()->show();

    // Take off the status bar
    setStatusBar(0);

    // Set the actions into the correct state
    enableForward(false);
    enableBackward(false);
    enableZoomToggle(false);
    enableZoomIn(false);
    enableZoomOut(false);

    // Connect to forward/backward status
    connect(&_oyabun, SIGNAL(forwardEnabled(bool)), this, SLOT(enableForward(bool)));
    connect(&_oyabun, SIGNAL(backwardEnabled(bool)), this, SLOT(enableBackward(bool)));

    /*
    // Connect to zoom status
    connect(&_pageDisplay, SIGNAL(zoomToggleEnabled(bool)), this, SLOT(enableZoomToggle(bool)));
    connect(&_pageDisplay, SIGNAL(zoomInEnabled(bool)), this, SLOT(enableZoomIn(bool)));
    connect(&_pageDisplay, SIGNAL(zoomOutEnabled(bool)), this, SLOT(enableZoomOut(bool)));
    */

    if (!initialArg.isEmpty()) {
        // Open the file right away
        if (QDir::isRelativePath(initialArg)) {
            _oyabun.start(QDir::current().absoluteFilePath(initialArg));
        } else {
            _oyabun.start(initialArg);
        }
        kDebug()<<"Page loader initialized";

        // Reflect the open file's name in the title bar
        setCaption(initialArg);
    }
}

Yomikata::~Yomikata()
{
}

void Yomikata::open()
{
    // Get an existing file from the user
    QString filename = KFileDialog::getOpenFileName(
        KUrl(),
        FileClassifier::getFileDialogWildcardString(),
        this,
        QString());

    if (filename.isEmpty()) {
        // If a file wasn't selected, don't do anything
        return;
    }

    _oyabun.start(filename);

    // Reflect the open file's name in the title bar
    setCaption(filename);
}

void Yomikata::wheelEvent(QWheelEvent *event)
{
    // Note: allows scrolling on menubar, similar to most other KDE apps

    // See if CTRL or SHIFT is being pressed
    bool zoom = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) != 0;

    if (zoom) {
        if (event->delta() > 0 && _zoomInAction->isEnabled()) {
            // Zoom in

        } else if (event->delta() < 0 && _zoomOutAction->isEnabled()) {
            // Zoom out

        } else {
            // Can't zoom
            event->ignore();
        }

    } else {
        if (event->delta() > 0 && _pageBackwardAction->isEnabled()) {
            // Page back
            _oyabun.turnPageBackward();

        } else if (event->delta() < 0 && _pageForwardAction->isEnabled()) {
            // Page forward
            _oyabun.turnPageForward();

        } else {
            // Can't page
            event->ignore();
        }
    }
}

void Yomikata::mousePressEvent(QMouseEvent *event)
{
    // Middle mouse triggers paging forward one page
    if (event->button() == Qt::MidButton && _pageForwardAction->isEnabled()) {
        _oyabun.turnPageForwardOnePage();
    }

    if (event->button() == Qt::RightButton && _zoomToggleAction->isEnabled()) {
    }
}

void Yomikata::setSinglePageView()
{
    Settings::setViewMode(SingleMode);
    Settings::self()->writeConfig();
}
void Yomikata::setComicsView()
{
    Settings::setViewMode(ComicsMode);
    Settings::self()->writeConfig();
}
void Yomikata::setMangaView()
{
    Settings::setViewMode(MangaMode);
    Settings::self()->writeConfig();
}

void Yomikata::enableForward(bool enabled)
{
    _pageForwardAction->setEnabled(enabled);
    _pageToEndAction->setEnabled(enabled);
    _pageLeftAction->setEnabled(enabled);
}
void Yomikata::enableBackward(bool enabled)
{
    _pageBackwardAction->setEnabled(enabled);
    _pageToStartAction->setEnabled(enabled);
    _pageRightAction->setEnabled(enabled);
}

void Yomikata::enableZoomToggle(bool enabled)
{
    _zoomToggleAction->setEnabled(enabled);
}
void Yomikata::enableZoomIn(bool enabled)
{
    _zoomInAction->setEnabled(enabled);
}
void Yomikata::enableZoomOut(bool enabled)
{
    _zoomOutAction->setEnabled(enabled);
}

void Yomikata::toggleFullScreen(bool checked)
{
    kDebug()<<"Fullcreen toggled"<<checked;
    if (checked) {
        showFullScreen();
        // Give warning about switching to fullscreen and losing the menubar
        //menuBar()->hide();
    } else {
        showNormal();
        menuBar()->show();
    }
}

void Yomikata::toggleMenubar(bool checked)
{
    if (checked) {
        menuBar()->show();
    } else {
        // Give warning about disabling the menubar
        menuBar()->hide();
    }
}

void Yomikata::createActions()
{
    // Quit
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    // Open file
    KStandardAction::open(this, SLOT(open()), actionCollection());

    // Toggle fullscreen
    KAction *fullScreenAction = KStandardAction::fullScreen(this, SLOT(toggleFullScreen(bool)), this, actionCollection());
    fullScreenAction->setShortcut(Qt::Key_F);

    // Show menubar
    KStandardAction::showMenubar(this, SLOT(toggleMenubar(bool)), actionCollection());

    // Page forward
    _pageForwardAction = actionCollection()->addAction( "page_forward" );
    _pageForwardAction->setText(i18n("Page &Forward"));
    connect(_pageForwardAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageForward()));
    _pageForwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Space<<Qt::Key_PageDown);
    addAction(_pageForwardAction);

    // Page backward
    _pageBackwardAction = actionCollection()->addAction( "page_backward" );
    _pageBackwardAction->setText(i18n("Page Backward"));
    connect(_pageBackwardAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageBackward()));
    _pageBackwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Backspace<<Qt::Key_PageUp);
    addAction(_pageBackwardAction);

    // Page left
    _pageLeftAction = new KAction(KIcon("go-previous"),  i18n("Page &Left"), this);
    _pageLeftAction->setShortcut(Qt::Key_Left);
    _pageLeftAction->setObjectName("page_left");
    connect(_pageLeftAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageForward()));
    actionCollection()->addAction(_pageLeftAction->objectName(), _pageLeftAction);

    // Page right
    _pageRightAction = new KAction(KIcon("go-next"),  i18n("Page &Right"), this);
    _pageRightAction->setShortcut(Qt::Key_Right);
    _pageRightAction->setObjectName("page_right");
    connect(_pageRightAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageBackward()));
    actionCollection()->addAction(_pageRightAction->objectName(), _pageRightAction);

    // Page to start
    _pageToStartAction = new KAction(KIcon("go-top"),  i18n("Go to &Start"), this);
    _pageToStartAction->setShortcut(Qt::Key_Home);
    _pageToStartAction->setObjectName("page_to_start");
    connect(_pageToStartAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageToStart()));
    actionCollection()->addAction(_pageToStartAction->objectName(), _pageToStartAction);

    // Page to end
    _pageToEndAction = new KAction(KIcon("go-bottom"),  i18n("Go to &End"), this);
    _pageToEndAction->setShortcut(Qt::Key_End);
    _pageToEndAction->setObjectName("page_to_end");
    connect(_pageToEndAction, SIGNAL(triggered(bool)), &_oyabun, SLOT(turnPageToEnd()));
    actionCollection()->addAction(_pageToEndAction->objectName(), _pageToEndAction);

    // Toggle zoom
    _zoomToggleAction = new KAction(KIcon("zoom-best-fit"),  i18n("&Toggle Zoom"), this);
    _zoomToggleAction->setShortcut(Qt::Key_Z);
    _zoomToggleAction->setObjectName("zoom_toggle");

    // Zoom in
    _zoomInAction = new KAction(KIcon("zoom-in"),  i18n("Zoom &In"), this);
    _zoomInAction->setShortcut(Qt::Key_C);
    _zoomInAction->setObjectName("zoom_in");

    // Zoom out
    _zoomOutAction = new KAction(KIcon("zoom-out"),  i18n("Zoom &Out"), this);
    _zoomOutAction->setShortcut(Qt::Key_X);
    _zoomOutAction->setObjectName("zoom_out");

    // View mode
    _viewSelectAction = new KSelectAction(i18n("View &Mode"), this);
    _singlePageAction = _viewSelectAction->addAction(i18n("&Single Page"));
    _singlePageAction->setObjectName("single_page");
    connect(_singlePageAction, SIGNAL(triggered(bool)), this, SLOT(setSinglePageView()));
    actionCollection()->addAction(_singlePageAction->objectName(), _singlePageAction);
    _comicsViewAction = _viewSelectAction->addAction(i18n("&Comics View"));
    _comicsViewAction->setObjectName("comics_view");
    connect(_comicsViewAction, SIGNAL(triggered(bool)), this, SLOT(setComicsView()));
    actionCollection()->addAction(_comicsViewAction->objectName(), _comicsViewAction);
    _mangaViewAction = _viewSelectAction->addAction(i18n("&Manga View"));
    _mangaViewAction->setObjectName("manga_view");
    connect(_mangaViewAction, SIGNAL(triggered(bool)), this, SLOT(setMangaView()));
    actionCollection()->addAction(_mangaViewAction->objectName(), _mangaViewAction);
    _viewSelectAction->setCurrentItem(Settings::viewMode());
}

void Yomikata::setAppDefaults()
{
    // Window size
    //resize(700, 400);

    // Menubar state
    menuBar()->show();

    // Toolbar state
    KToolBar *mainToolBar = toolBar("mainToolBar");
    Q_ASSERT(mainToolBar != 0);
    mainToolBar->hide();
}

#include "yomikata.moc"
