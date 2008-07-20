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
#include <QDir>

#include "yomikata.h"
#include "fileinfo.h"
#include "testwidget.h"

Yomikata::Yomikata(const QString &initialArg, QWidget *parent)
    :KMainWindow(parent), _fileDecodeThread(&_pageCache), _extractDecodeThread(&_pageCache)
{
    // Set initial state
    _pageMode = MangaMode;
    //_pageMode = SingleMode;

    // Create the widget for displaying the pages
    setCentralWidget(&_pageDisplay);

    //setCentralWidget(new TestWidget);

    // Set up the actions (open, quit, etc.)
    createActions();

    // Not much point in a small window
    setAppDefaults();//setMinimumSize(200, 200);

    // Load default toolbars, shortcuts
    setupGUI();
    //menuBar()->show();

    // Set the actions into the correct state
    enableForward(false);
    enableBackward(false);
    enableZoomToggle(false);
    enableZoomIn(false);
    enableZoomOut(false);

    // Connect to page decode failure
    //connect(&_pageCache, SIGNAL(pageReadFailed(int)), this, SLOT(pageReadFailed(int)));

    // Connect to forward/backward status
    connect(&_pageCache, SIGNAL(forwardEnabled(bool)), this, SLOT(enableForward(bool)));
    connect(&_pageCache, SIGNAL(backwardEnabled(bool)), this, SLOT(enableBackward(bool)));

    // Connect to display signals
    connect(&_pageCache, SIGNAL(showOnePage(const QPixmap &, int, int)), &_pageDisplay, SLOT(setOnePage(const QPixmap &, int, int)));
    connect(&_pageCache, SIGNAL(showTwoPages(const QPixmap &, const QPixmap &, int, int, int)), &_pageDisplay, SLOT(setTwoPages(const QPixmap &, const QPixmap &, int, int, int)));
    connect(&_pageCache, SIGNAL(showPageNumber(int, int, int)), &_pageDisplay, SLOT(setPageNumber(int, int, int)));

    // Connect display size changes
    connect(&_pageDisplay, SIGNAL(displaySizeChanged(const QSize &)), &_pageCache, SLOT(setDisplaySize(const QSize &)));

    // Connect to zoom status
    connect(&_pageDisplay, SIGNAL(zoomToggleEnabled(bool)), this, SLOT(enableZoomToggle(bool)));
    connect(&_pageDisplay, SIGNAL(zoomInEnabled(bool)), this, SLOT(enableZoomIn(bool)));
    connect(&_pageDisplay, SIGNAL(zoomOutEnabled(bool)), this, SLOT(enableZoomOut(bool)));

    // Connect to the listers
    connect(&_fileLister, SIGNAL(listBuilt(int, const QStringList &)),
            &_pageCache, SLOT(initialize(int, const QStringList &)));
    connect(&_extractLister, SIGNAL(listBuilt(int, const QStringList &)),
            &_pageCache, SLOT(initialize(int, const QStringList &)));

    _pageCache.setDisplaySize(_pageDisplay.size());

    if (!initialArg.isEmpty()) {
        // Open the file right away
        if (QDir::isRelativePath(initialArg)) {
            startListing(QDir::current().absoluteFilePath(initialArg));
        } else {
            startListing(initialArg);
        }
        kDebug()<<"Page loader initialized"<<endl;

        // Reflect the open file's name in the title bar
        setCaption(initialArg);
    }
}

Yomikata::~Yomikata()
{
    // Stop the threads
    _extractDecodeThread.stop();
    _fileDecodeThread.stop();

    // Stop scheduling
    _pageCache.reset();

    // Wait for the threads
    _extractDecodeThread.wait();
    _fileDecodeThread.wait();
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

    startListing(filename);

    // Reflect the open file's name in the title bar
    setCaption(filename);

    // Show the waiting cursor
    _pageDisplay.loadingStarted();
}

void Yomikata::startListing(const QString &initialFile)
{
    KUrl path = KUrl::fromPath(initialFile);

    // Check if we're opening plain files or an archive
    if (FileInfo::isImageFile(initialFile)) {
        _archiveMode = false;

        // Start the directory listing
        _fileLister.list(initialFile);

    } else if (FileInfo::isArchiveFile(initialFile)) {
        _archiveMode = true;
        _archivePath = initialFile;
        _archiveType = FileInfo::getArchiveType(_archivePath);

        // Start the archive listing
        _extractLister.list(_archiveType, _archivePath);
    } else {
        Q_ASSERT(false);
    }

    kDebug()<<"Listing started"<<endl;

    // Stop the threads
    _extractDecodeThread.stop();
    _fileDecodeThread.stop();

    // Stop scheduling
    _pageCache.reset();

    // Wait for the threads
    _extractDecodeThread.wait();
    _fileDecodeThread.wait();

    // Start the decode threads
    if (_archiveMode) {
        _extractDecodeThread.setArchive(_archiveType, _archivePath);
        _extractDecodeThread.start();
    } else {
        _fileDecodeThread.start();
    }
}

void Yomikata::pageReadFailed(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" failed reading"<<endl;

    KMessageBox::error(this, i18n("Cannot load the image."));
}

void Yomikata::wheelEvent(QWheelEvent *event)
{
    // Note: allows scrolling on menubar, similar to most other KDE apps

    // See if CTRL or SHIFT is being pressed
    bool zoom = (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) != 0;

    if (zoom) {
        if (event->delta() > 0 && _zoomInAction->isEnabled()) {
            // Zoom in
            _pageDisplay.zoomIn();

        } else if (event->delta() < 0 && _zoomOutAction->isEnabled()) {
            // Zoom out
            _pageDisplay.zoomOut();

        } else {
            // Can't zoom
            event->ignore();
        }

    } else {
        if (event->delta() > 0 && _pageBackwardAction->isEnabled()) {
            // Page back
            _pageCache.navigateBackward();

        } else if (event->delta() < 0 && _pageForwardAction->isEnabled()) {
            // Page forward
            _pageCache.navigateForward();

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
        _pageCache.navigateForwardOnePage();
    }

    if (event->button() == Qt::RightButton && _zoomToggleAction->isEnabled()) {
        _pageDisplay.toggleZoom();
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
        menuBar()->hide();
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
    connect(_pageForwardAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateForward()));
    _pageForwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Space<<Qt::Key_PageDown);
    addAction(_pageForwardAction);

    // Page backward
    _pageBackwardAction = actionCollection()->addAction( "page_backward" );
    _pageBackwardAction->setText(i18n("Page Backward"));
    connect(_pageBackwardAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateBackward()));
    _pageBackwardAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Backspace<<Qt::Key_PageUp);
    addAction(_pageBackwardAction);

    // Page left
    _pageLeftAction = new KAction(KIcon("go-previous"),  i18n("Page &Left"), this);
    _pageLeftAction->setShortcut(Qt::Key_Left);
    _pageLeftAction->setObjectName("page_left");
    connect(_pageLeftAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateForward()));
    actionCollection()->addAction(_pageLeftAction->objectName(), _pageLeftAction);

    // Page right
    _pageRightAction = new KAction(KIcon("go-next"),  i18n("Page &Right"), this);
    _pageRightAction->setShortcut(Qt::Key_Right);
    _pageRightAction->setObjectName("page_right");
    connect(_pageRightAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateBackward()));
    actionCollection()->addAction(_pageRightAction->objectName(), _pageRightAction);

    // Page to start
    _pageToStartAction = new KAction(KIcon("go-top"),  i18n("Go to &Start"), this);
    _pageToStartAction->setShortcut(Qt::Key_Home);
    _pageToStartAction->setObjectName("page_to_start");
    connect(_pageToStartAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateToStart()));
    actionCollection()->addAction(_pageToStartAction->objectName(), _pageToStartAction);

    // Page to end
    _pageToEndAction = new KAction(KIcon("go-bottom"),  i18n("Go to &End"), this);
    _pageToEndAction->setShortcut(Qt::Key_End);
    _pageToEndAction->setObjectName("page_to_end");
    connect(_pageToEndAction, SIGNAL(triggered(bool)), &_pageCache, SLOT(navigateToEnd()));
    actionCollection()->addAction(_pageToEndAction->objectName(), _pageToEndAction);

    // Toggle zoom
    _zoomToggleAction = new KAction(KIcon("zoom-best-fit"),  i18n("&Toggle Zoom"), this);
    _zoomToggleAction->setShortcut(Qt::Key_Z);
    _zoomToggleAction->setObjectName("zoom_toggle");
    connect(_zoomToggleAction, SIGNAL(triggered(bool)), &_pageDisplay, SLOT(toggleZoom()));
    actionCollection()->addAction(_zoomToggleAction->objectName(), _zoomToggleAction);

    // Zoom in
    _zoomInAction = new KAction(KIcon("zoom-in"),  i18n("Zoom &In"), this);
    _zoomInAction->setShortcut(Qt::Key_C);
    _zoomInAction->setObjectName("zoom_in");
    connect(_zoomInAction, SIGNAL(triggered(bool)), &_pageDisplay, SLOT(zoomIn()));
    actionCollection()->addAction(_zoomInAction->objectName(), _zoomInAction);

    // Zoom out
    _zoomOutAction = new KAction(KIcon("zoom-out"),  i18n("Zoom &Out"), this);
    _zoomOutAction->setShortcut(Qt::Key_X);
    _zoomOutAction->setObjectName("zoom_out");
    connect(_zoomOutAction, SIGNAL(triggered(bool)), &_pageDisplay, SLOT(zoomOut()));
    actionCollection()->addAction(_zoomOutAction->objectName(), _zoomOutAction);
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
