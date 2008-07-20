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

#include "yomikata.h"

/**
 * @TODO: Read files in a directory
 * @TODO: Display two pages
 * @TODO: Keyboard shortcuts for changing pages
 * @TODO: Fullscreen mode
 * @TODO: Read archives
 * @TODO: Buttons at ends of archives to jump to next ones
 * @TODO: Optimize archive retrival (stdout)
 * @TODO: Optimize archive retrival (pipeline)
 * @TODO: Allow rotated output
 * @TODO: Thumbnails
 * @TODO: Check out pixmap cache
 * @TODO: Zoom tool (free roaming, scroll to zoom)
 * @TODO: Give a good colour to the spacer panels
 * @TODO: Open image in external editor
 * @TODO: Open with commandline-parameters
 * @TODO: Copy non-local file when given url
 * @TODO: Use mimetype filters for file open dialog
 * _maybe_ use zlib
 * _maybe_ use kioslaves
 * _maybe_ option to convert an archive
 * _maybe_ recursive openning
 */

Yomikata::Yomikata(QWidget *parent) : KMainWindow(parent)
{
    setMinimumSize(200, 200);

    // Create the dummy widget that holds the page layout
    _innerWidget = new QWidget;

    // Create the spacer widgets
    _spacerA = new QWidget;
    _spacerB = new QWidget;

    // Create the label for displaying the image
    _imageLabel = new QLabel;
    _imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _imageLabel->setScaledContents(true);

    // Set up the page layout
    _pageLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    _pageLayout->setMargin(0);
    _pageLayout->setSpacing(0);
    _pageLayout->addWidget(_spacerA);
    _pageLayout->addWidget(_imageLabel);
    _pageLayout->addWidget(_spacerB);

    // Put the page layout as the only thing in the window
    _innerWidget->setLayout(_pageLayout);
    setCentralWidget(_innerWidget);

    // Set up the actions (open, quit, etc.)
    setupActions();
}

void Yomikata::resizeEvent(QResizeEvent *)
{
    // If there are pages being displayed, position them correctly
    if (_imageLabel->pixmap() != 0) {
        QSize totalSize = _innerWidget->size();
        QSize imageSize = _imageLabel->pixmap()->size();

        // Scale the page so it fits snug in the window
        imageSize.scale(totalSize, Qt::KeepAspectRatio);

        // Check which dimension has extra space and adjust the
        //  weights of the image and spacers accordingly
        if (imageSize.height() == totalSize.height()) {
            _pageLayout->setDirection(QBoxLayout::LeftToRight);

            _pageLayout->setStretchFactor(_imageLabel, imageSize.width());

            _pageLayout->setStretchFactor(_spacerA,
                (totalSize.width() - imageSize.width()) / 2);
            _pageLayout->setStretchFactor(_spacerB,
                (totalSize.width() - imageSize.width() + 1) / 2);
        } else {
            _pageLayout->setDirection(QBoxLayout::TopToBottom);

            _pageLayout->setStretchFactor(_imageLabel, imageSize.height());

            _pageLayout->setStretchFactor(_spacerA,
                (totalSize.height() - imageSize.height()) / 2);
            _pageLayout->setStretchFactor(_spacerB,
                (totalSize.height() - imageSize.height() + 1) / 2);
        }

        // Force the layout to change before the window is drawn
        _pageLayout->activate();
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
        "*|All Files"
        ),
        this,
        QString());

    if (filename.isEmpty()) {
        // If a file wasn't selected, don't do anything
        return;
    }

    connect(&_pageLoader, SIGNAL(pagesListed(int, int)), this, SLOT(pagesListed(int, int)));
    connect(&_pageLoader, SIGNAL(pageRead(int)), this, SLOT(pageRead(int)));
    connect(&_pageLoader, SIGNAL(pageReadFailed(int)), this, SLOT(pageReadFailed(int)));
    _pageLoader.initialize(filename);
    kDebug()<<"Page loader initialized"<<endl;

    // Reflect the open file's name in the title bar
    setCaption(filename);
}

void Yomikata::pagesListed(int, int)
{
    kDebug()<<"Pages listed"<<endl;
}

void Yomikata::pageRead(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" read"<<endl;

    // Display the image
    _imageLabel->setPixmap(_pageLoader.getPage(pageNum));

    // Scale it correctly
    resizeEvent(0);
}

void Yomikata::pageReadFailed(int pageNum)
{
    kDebug()<<"Page "<<pageNum<<" failed reading"<<endl;

    //KMessageBox::error(this, i18n("Cannot load the image:\n%1.").arg(filename));
    KMessageBox::error(this, i18n("Cannot load the image."));
}

void Yomikata::setupActions()
{
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());

    setupGUI();
}

#include "yomikata.moc"
