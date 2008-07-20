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

/**
 * Done: Read files in a directory
 * Done: Keyboard shortcuts for changing pages
 * Done: Optimized window resizing
 * Done: Status of page # / total
 * Done: Fullscreen mode
 * Done: Read archives
 * @TODO: Display two pages
 * @TODO: Open other archive formats
 * @TODO: Open more than once
 * @TODO: Preempt the other queued jobs with the current page using QueuePolicy
 * @TODO: Buttons at ends of archives to jump to next ones
 * @TODO: Memory use awareness (check out pixmap cache)
 * @TODO: Hide mouse after idle time
 * @TODO: Zoom tool (free roaming, scroll to zoom)
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
    _numPages = 0;
    _targetPage = -1;
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
    synchronizeActions();

    // Connect to the page loader
    connect(&_pageLoader, SIGNAL(pagesListed(int, int)), this, SLOT(pagesListed(int, int)), Qt::QueuedConnection);
    connect(&_pageLoader, SIGNAL(pageRead(int)), this, SLOT(pageRead(int)), Qt::QueuedConnection);
    connect(&_pageLoader, SIGNAL(pageReadFailed(int)), this, SLOT(pageReadFailed(int)), Qt::QueuedConnection);

    _pageLoader.setDisplaySize(_pageDisplay->size());

    _pageNumberLabel = 0;
}

void Yomikata::resizeEvent(QResizeEvent *)
{
    // Check that we have something open
    if (_targetPage != -1) {
        // Notify the page loader about the new display size
        _pageLoader.setDisplaySize(_pageDisplay->size());

        // By resizing, the zoom mode is activated
        // If we are starting zoom mode right now, get an unscaled version of
        //  the pages being displayed
        if (!_pageLoader.isZoomMode()) {
            _pageLoader.startZoomMode();

            // Get the correct size of pixmap
            _pageLoader.startReadingPage(_targetPage);
            if (_pageMode != SingleMode && _targetPageB != -1) {
                _pageLoader.startReadingPage(_targetPageB);
            }
        }
    }

    if (_pageNumberLabel != 0) {
        // Don't move the label, just destroy it
        _pageNumberTimer.stop();

        delete _pageNumberLabel;
        _pageNumberLabel = 0;
    }
}

void Yomikata::open()
{
    // Get an existing file from the user
    QString filename = KFileDialog::getOpenFileName(
        KUrl(),
        QString(
        "*.jpg *.rar|All Supported Files\n"
        "*.jpg|Image Files\n"
        "*.rar|Archive Files\n"
        //"*|All Files"
        ),
        this,
        QString());

    if (filename.isEmpty()) {
        // If a file wasn't selected, don't do anything
        return;
    }

    _numPages = 0;
    _targetPage = -1;
    _targetPageB = -1;
    synchronizeActions();

    _pageLoader.initialize(filename);
    kDebug()<<"Page loader initialized"<<endl;

    // Reflect the open file's name in the title bar
    setCaption(filename);
}

void Yomikata::pagesListed(int initialPosition, int numPages)
{
    kDebug()<<"Pages listed: initial position "<<initialPosition<<", total pages "<<numPages<<endl;

    _numPages = numPages;
    _targetPage = initialPosition;
    if (_pageMode != SingleMode && _targetPage + 1 < _numPages) {
        _targetPageB = initialPosition + 1;
    }

    changePage();

    synchronizeActions();
}

void Yomikata::pageRead(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" read"<<endl;

    if (_targetPageB == -1) {
        if (pageNum == _targetPage) {
            _pageDisplay->setOnePage(_pageLoader.getPage(_targetPage));
        }
    } else {
        if ((pageNum == _targetPage && _pageLoader.isPageRead(_targetPageB))
            || (pageNum == _targetPageB && _pageLoader.isPageRead(_targetPage))) {
            _pageDisplay->setTwoPages(_pageLoader.getPage(_targetPageB), _pageLoader.getPage(_targetPage));
        }
    }
}

void Yomikata::pageReadFailed(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" failed reading"<<endl;

    //KMessageBox::error(this, i18n("Cannot load the image:\n%1.").arg(filename));
    KMessageBox::error(this, i18n("Cannot load the image."));
}

void Yomikata::changePage()
{
    kDebug()<<"_targetPage "<<_targetPage<<", _targetPageB "<<_targetPageB<<endl;

    // Changing the page stops zoom mode
    if (_pageLoader.isZoomMode()) {
        _pageLoader.stopZoomMode(_targetPage);
    }

    if (_pageMode == SingleMode || _targetPageB == -1) {
        if (_pageLoader.isPageRead(_targetPage)) {
            // If there's something decoded, display it
            _pageDisplay->setOnePage(_pageLoader.getPage(_targetPage));

            if (!_pageLoader.isPageScaled(_targetPage)) {
                // If the decoded page isn't the right size, schedule the decoding of a
                //  correctly sized page
                kDebug()<<"Wrong size page loaded"<<endl;
                _pageLoader.startReadingPage(_targetPage);
            }
        } else {
            _pageLoader.startReadingPage(_targetPage);
        }
    } else {
        bool pageRead = _pageLoader.isPageRead(_targetPage);
        bool pageReadB = _pageLoader.isPageRead(_targetPageB);

        if (pageRead && pageReadB) {
            _pageDisplay->setTwoPages(_pageLoader.getPage(_targetPageB), _pageLoader.getPage(_targetPage));

            if (!_pageLoader.isPageScaled(_targetPage)) {
                // If the decoded page isn't the right size, schedule the decoding of a
                //  correctly sized page
                kDebug()<<"Wrong size page loaded"<<endl;
                _pageLoader.startReadingPage(_targetPage);
            }
            if (!_pageLoader.isPageScaled(_targetPageB)) {
                // If the decoded page isn't the right size, schedule the decoding of a
                //  correctly sized page
                kDebug()<<"Wrong size page loaded"<<endl;
                _pageLoader.startReadingPage(_targetPageB);
            }
        } else {
            if (!pageRead) {
                _pageLoader.startReadingPage(_targetPage);
            }
            if (!pageReadB) {
                _pageLoader.startReadingPage(_targetPageB);
            }
        }
    }

    synchronizeActions();

    // Start the timer to hide the page number
    // Restart the timer if it had been started
    if (_pageNumberTimer.isActive()) {
        _pageNumberTimer.stop();
    }
    const int PAGE_NUMBER_HIDE_DELAY = 1500;
    _pageNumberTimer.start(PAGE_NUMBER_HIDE_DELAY, this);

    // Show the page number
    if (_pageNumberLabel == 0) {
        _pageNumberLabel = new QLabel(_pageDisplay);
        _pageNumberLabel->hide();
        _pageNumberLabel->setBackgroundRole(QPalette::Window);
        _pageNumberLabel->setAutoFillBackground(true);
        _pageNumberLabel->setAlignment(Qt::AlignCenter);
    }

    _pageNumberLabel->setUpdatesEnabled(false);
    const int WIDTH_PADDING = 20;
    QString text;
    if (_targetPageB == -1) {
        text = QString("%1 / %2").arg(QString::number(_targetPage + 1), QString::number(_numPages));
    } else {
        text = QString("%1-%2 / %3").arg(QString::number(_targetPage + 1), QString::number(_targetPageB + 1), QString::number(_numPages));
    }
    QFontMetrics metrics(_pageNumberLabel->font());
    _pageNumberLabel->resize(metrics.width(text) + WIDTH_PADDING, _pageNumberLabel->height());
    _pageNumberLabel->setText(text);
    _pageNumberLabel->move(_pageDisplay->width() - _pageNumberLabel->width(), _pageDisplay->height() - _pageNumberLabel->height());
    _pageNumberLabel->show();
    _pageNumberLabel->raise();
    _pageNumberLabel->setUpdatesEnabled(true);
}

void Yomikata::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == _pageNumberTimer.timerId());
    Q_ASSERT(_pageNumberLabel != 0);

    _pageNumberTimer.stop();

    delete _pageNumberLabel;
    _pageNumberLabel = 0;
}

void Yomikata::wheelEvent(QWheelEvent *event)
{
    // Note: allows scrolling on menubar, similar to most other KDE apps
    if (event->delta() > 0 && _pageRightAction->isEnabled()) {
        // Page back
        pageRight();
    } else if (event->delta() < 0 && _pageLeftAction->isEnabled()) {
        // Page forward
        pageLeft();
    } else {
        // Can't page
        event->ignore();
    }
}

void Yomikata::pageLeft()
{
    kDebug()<<"Paging left"<<endl;

    if (_targetPageB == -1) {
        _targetPage++;
    } else {
        _targetPage += 2;
    }

    if (_pageMode != SingleMode && _targetPage + 1 < _numPages) {
        _targetPageB = _targetPage + 1;
    } else {
        _targetPageB = -1;
    }

    changePage();
}

void Yomikata::pageRight()
{
    kDebug()<<"Paging right"<<endl;

    if (_pageMode == SingleMode) {
        _targetPage--;
    } else {
        if (_targetPage - 2 < 0) {
            _targetPage--;
            _targetPageB = -1;
        } else {
            _targetPage -= 2;
            _targetPageB = _targetPage + 1;
        }
    }

    changePage();
}

void Yomikata::synchronizeActions()
{
    if (_numPages == 0) {
        _pageLeftAction->setEnabled(false);
        _pageRightAction->setEnabled(false);
    } else {
        _pageRightAction->setEnabled(_targetPage > 0);
        if (_targetPageB == -1) {
            _pageLeftAction->setEnabled(_targetPage < _numPages - 1);
        } else {
            _pageLeftAction->setEnabled(_targetPageB < _numPages - 1);
        }
    }
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

    // Page left
    _pageLeftAction = new KAction(KIcon("previous"),  i18n("Page &Left"), this);
    _pageLeftAction->setShortcut(Qt::Key_Left);
    _pageLeftAction->setObjectName("page_left");
    connect(_pageLeftAction, SIGNAL(triggered(bool)), SLOT(pageLeft()));
    actionCollection()->addAction(_pageLeftAction->objectName(), _pageLeftAction);

    // Page right
    _pageRightAction = new KAction(KIcon("next"),  i18n("Page &Right"), this);
    _pageRightAction->setShortcut(Qt::Key_Right);
    _pageRightAction->setObjectName("page_right");
    connect(_pageRightAction, SIGNAL(triggered(bool)), SLOT(pageRight()));
    actionCollection()->addAction(_pageRightAction->objectName(), _pageRightAction);
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
