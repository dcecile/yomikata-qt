#include "mainwindowkde.h"

#include <KMenuBar>
#include <KFileDialog>
#include <KLocalizedString>
#include <KStandardAction>
#include <KActionCollection>
#include <KToggleFullScreenAction>
#include <KToolBar>

#include "debug.h"

MainWindowKde::MainWindowKde(const QString &initialArg, QWidget *parent)
    : MainWindow(initialArg, parent)
{
    // Start opening the initial file
    if (!initialArg.isEmpty())
    {
        // Reflect the open file's name in the title bar
        setCaption(initialArg);
    }

    // Set up the actions (open, quit, etc.)
    createActions();

    // Not much point in a small window
    setAppDefaults();//setMinimumSize(200, 200);

    // Load default toolbars, shortcuts
    setupGUI();
    menuBar()->show();

    // Take off the status bar
    setStatusBar(NULL);

    // Connect to forward/backward status
    //connect(&_book, SIGNAL(forwardEnabled(bool)), this, SLOT(enableForward(bool)));
    //connect(&_book, SIGNAL(backwardEnabled(bool)), this, SLOT(enableBackward(bool)));

}

MainWindowKde::~MainWindowKde()
{
}

void MainWindowKde::open()
{
    // Get an existing file from the user
    QString filename = KFileDialog::getOpenFileName(
            KUrl(),
            "*.*",//FileClassification::getFileDialogWildcardString(),
            this,
            QString());

    if (filename.isEmpty())
    {
        // If a file wasn't selected, don't do anything
        return;
    }

    setSource(filename);

    // Reflect the open file's name in the title bar
    setCaption(filename);
}

/**
 * @todo Give warning about switching to fullscreen and losing the menubar
 * @todo Remember menubar settings
 * @todo Do something fancy with the toolbar
 */
void MainWindowKde::toggleFullScreen(bool checked)
{
    debug()<<"Fullcreen toggled"<<checked;

    if (checked)
    {
        showFullScreen();
        menuBar()->hide();
    }
    else
    {
        showNormal();
        menuBar()->show();
    }
}

void MainWindowKde::toggleMenubar(bool checked)
{
    if (checked)
    {
        menuBar()->show();
    }
    else
    {
        // Give warning about disabling the menubar
        menuBar()->hide();
    }
}


void MainWindowKde::createActions()
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
    //connect(_pageForwardAction, SIGNAL(triggered(bool)), &_book, SLOT(turnPageForward()));
    _pageForwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Space<<Qt::Key_PageDown);
    addAction(_pageForwardAction);

    // Page backward
    _pageBackwardAction = actionCollection()->addAction( "page_backward" );
    _pageBackwardAction->setText(i18n("Page Backward"));
    //connect(_pageBackwardAction, SIGNAL(triggered(bool)), &_book, SLOT(turnPageBackward()));
    _pageBackwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Backspace<<Qt::Key_PageUp);
    addAction(_pageBackwardAction);

    // Page left
    _pageLeftAction = new KAction(KIcon("go-previous"),  i18n("Page &Left"), this);
    _pageLeftAction->setShortcut(Qt::Key_Left);
    _pageLeftAction->setObjectName("page_left");
    //connect(_pageLeftAction, SIGNAL(triggered(bool)), &_book, SLOT(turnPageForward()));
    actionCollection()->addAction(_pageLeftAction->objectName(), _pageLeftAction);

    // Page right
    _pageRightAction = new KAction(KIcon("go-next"),  i18n("Page &Right"), this);
    _pageRightAction->setShortcut(Qt::Key_Right);
    _pageRightAction->setObjectName("page_right");
    //connect(_pageRightAction, SIGNAL(triggered(bool)), &_book, SLOT(turnPageBackward()));
    actionCollection()->addAction(_pageRightAction->objectName(), _pageRightAction);

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
}

void MainWindowKde::setAppDefaults()
{
    // Menubar state
    menuBar()->show();

    // Toolbar state
    KToolBar *mainToolBar = toolBar("mainToolBar");
    Q_ASSERT(mainToolBar != 0);
    mainToolBar->hide();
}

#include "mainwindowkde.moc"
