#include <KApplication>
#include <KAction>
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

#include "yomikata.h"
#include "fileinfo.h"

/**
 * Done: Read files in a directory
 * Done: Keyboard shortcuts for changing pages
 * Done: Optimized window resizing
 * Done: Status of page # / total
 * Done: Fullscreen mode
 * Done: Read archives
 * Done: Display two pages
 * @TODO: Open other archive formats
 * @TODO: Open more than once
 * @TODO: Preempt the other queued jobs with the current page using QueuePolicy
 * @TODO: Buttons at ends of archives to jump to next ones
 * @TODO: Memory use awareness (check out pixmap cache)
 * @TODO: Hide mouse after idle time
 * @TODO: Zoom tool (free roaming, scroll to zoom)
 * @TODO: Correctly calculate double-page size for portait window
 * @TODO: Thumbnails
 * @TODO: Give a good colour to the spacer panels
 * @TODO: Open image in external editor
 * @TODO: Open with commandline-parameters
 * @TODO: Copy non-local file when given url
 * @TODO: Use mimetype filters for file open dialog
 * @TODO: All navigation shortcuts
 * @TODO: Viewing modes (single, left-right, right-left)
 * @TODO: Status bar for buffer fullness
 * @TODO: Status bar error messages
 * @TODO: Show plain text info files
 * @TODO: During resizing, pull out of zoom mode if the size doesn't change for a while
 * @TODO: Allow rotated output
 * @TODO: Fullscreen mode w/o menubar
 * @TODO: Handle erroraneous input
 * @TODO: Bookmarks, maybe autoopen at previous position
 * @TODO: Session management
 * _maybe_: Optimize archive retrival (stdout)
 * _maybe: Optimize archive retrival (pipeline)
 * _mabye_: Status bar for page # / total
 * _maybe_ use zlib
 * _maybe_ use kioslaves
 * _maybe_ option to convert an archive
 * _maybe_ recursive openning
 * _maybe_ draw pixmaps on QGLWidget
 */

Yomikata::Yomikata(QWidget *parent)
    :KMainWindow(parent)
{
    // Set initial state
    _pageMode = MangaMode;
    //_pageMode = SingleMode;

    // Create the widget for displaying the pages
    _pageDisplay = new PageDisplay;
    setCentralWidget(_pageDisplay);

    // Set up the actions (open, quit, etc.)
    createActions();

    // Not much point in a small window
    setAppDefaults();//setMinimumSize(200, 200);

    // Load default toolbars, shortcuts
    setupGUI();

    // Set the actions into the correct state
    enableForward(_pageLoader.isForwardEnabled());
    enableBackward(_pageLoader.isBackwardEnabled());

    // Connect to the page loader
    connect(&_pageLoader, SIGNAL(pageReadFailed(int)), this, SLOT(pageReadFailed(int)));

    connect(&_pageLoader, SIGNAL(forwardEnabled(bool)), this, SLOT(enableForward(bool)));
    connect(&_pageLoader, SIGNAL(backwardEnabled(bool)), this, SLOT(enableBackward(bool)));

    connect(&_pageLoader, SIGNAL(showOnePage(const QPixmap &, int, int)), _pageDisplay, SLOT(setOnePage(const QPixmap &, int, int)));
    connect(&_pageLoader, SIGNAL(showTwoPages(const QPixmap &, const QPixmap &, int, int, int)), _pageDisplay, SLOT(setTwoPages(const QPixmap &, const QPixmap &, int, int, int)));
    connect(&_pageLoader, SIGNAL(showPageNumber(int, int, int)), _pageDisplay, SLOT(setPageNumber(int, int, int)));

    connect(_pageDisplay, SIGNAL(displaySizeChanged(const QSize &)), &_pageLoader, SLOT(setDisplaySize(const QSize &)));

    _pageLoader.setDisplaySize(_pageDisplay->size());
}

void Yomikata::open()
{
    // Get an existing file from the user
    QString filename = KFileDialog::getOpenFileName(
        KUrl(),
        FileInfo::getFileDialogWildcardString(),
        this,
        QString());

    if (filename.isEmpty()) {
        // If a file wasn't selected, don't do anything
        return;
    }

    _pageLoader.initialize(filename);
    kDebug()<<"Page loader initialized"<<endl;

    // Reflect the open file's name in the title bar
    setCaption(filename);
}

void Yomikata::pageReadFailed(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" failed reading"<<endl;

    KMessageBox::error(this, i18n("Cannot load the image."));
}

void Yomikata::wheelEvent(QWheelEvent *event)
{
    // Note: allows scrolling on menubar, similar to most other KDE apps
    if (event->delta() > 0 && _pageBackwardAction->isEnabled()) {
        // Page back
        _pageLoader.navigateBackward();
    } else if (event->delta() < 0 && _pageForwardAction->isEnabled()) {
        // Page forward
        _pageLoader.navigateForward();
    } else {
        // Can't page
        event->ignore();
    }
}

void Yomikata::mousePressEvent(QMouseEvent *event)
{
    // Middle mouse triggers paging forward one page
    if (event->button() == Qt::MidButton && _pageForwardAction->isEnabled()) {
        _pageLoader.navigateForwardOnePage();
    }
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

void Yomikata::toggleFullScreen(bool checked)
{
    kDebug()<<"Fullcreen toggled "<<checked<<endl;
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
        //menuBar()->hide();
    }
}

void Yomikata::createActions()
{
    // Quit
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

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
    connect(_pageForwardAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateForward()));
    _pageForwardAction->setShortcut(Qt::Key_Space);
    addAction(_pageForwardAction);

    // Page backward
    _pageBackwardAction = actionCollection()->addAction( "page_backward" );
    _pageBackwardAction->setText(i18n("Page Backward"));
    connect(_pageBackwardAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateBackward()));
    _pageBackwardAction->setShortcut(Qt::Key_Backspace);
    addAction(_pageBackwardAction);

    // Page left
    _pageLeftAction = new KAction(KIcon("previous"),  i18n("Page &Left"), this);
    _pageLeftAction->setShortcut(Qt::Key_Left);
    _pageLeftAction->setObjectName("page_left");
    connect(_pageLeftAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateForward()));
    actionCollection()->addAction(_pageLeftAction->objectName(), _pageLeftAction);

    // Page right
    _pageRightAction = new KAction(KIcon("next"),  i18n("Page &Right"), this);
    _pageRightAction->setShortcut(Qt::Key_Right);
    _pageRightAction->setObjectName("page_right");
    connect(_pageRightAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateBackward()));
    actionCollection()->addAction(_pageRightAction->objectName(), _pageRightAction);

    // Page to start
    _pageToStartAction = new KAction(KIcon("top"),  i18n("Go to &Start"), this);
    _pageToStartAction->setShortcut(Qt::Key_Home);
    _pageToStartAction->setObjectName("page_to_start");
    connect(_pageToStartAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateToStart()));
    actionCollection()->addAction(_pageToStartAction->objectName(), _pageToStartAction);

    // Page to end
    _pageToEndAction = new KAction(KIcon("bottom"),  i18n("Go to &End"), this);
    _pageToEndAction->setShortcut(Qt::Key_End);
    _pageToEndAction->setObjectName("page_to_end");
    connect(_pageToEndAction, SIGNAL(triggered(bool)), &_pageLoader, SLOT(navigateToEnd()));
    actionCollection()->addAction(_pageToEndAction->objectName(), _pageToEndAction);
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
