#include "oyabun.h"

#include <KDebug>

#include "page.h"

Oyabun::Oyabun(QWidget *parent)
    : QObject(parent), _source(this), _fileList(_source, this), _book(this), _decoder(_source, this), _depicter(parent)
{
    // Connect to buka
    connect(&_fileList, SIGNAL(donePopulating(const vector<FileInfo> &)), this, SLOT(donePopulatingFileList(const vector<FileInfo> &)));
    connect(&_decoder, SIGNAL(decodeDone()), this, SLOT(decodeDone()));
    connect(&_depicter, SIGNAL(displayAreaChanged(const QSize &)), &_book, SLOT(resetDisplayArea(const QSize &)));
    connect(&_book, SIGNAL(currentPagesReset()), this, SLOT(currentPagesReset()));
    connect(&_book, SIGNAL(currentPagesReplanned()), &_depicter, SLOT(currentPagesReplanned()));

    // Set initial state
    _forwardEnabled = false;
    _backwardEnabled = false;
    _requestTime[0].start();
    _requestTime[1].start();
}

void Oyabun::enableForward(bool enabled)
{
    if (_forwardEnabled != enabled) {
        _forwardEnabled = enabled;
        emit forwardEnabled(enabled);
    }
}

void Oyabun::enableBackward(bool enabled)
{
    if (_backwardEnabled != enabled) {
        _backwardEnabled = enabled;
        emit backwardEnabled(enabled);
    }
}

void Oyabun::start(const QString &initialFile)
{
    kDebug()<<"Starting "<<initialFile;

    // Disable navigation
    enableForward(false);
    enableBackward(false);

    // Set the source file
    _source.set(initialFile);

    // Start populating the file list
    _fileList.retrieve();

    // Initialize the decoder
    _decoder.intialize();

    // Wait for the file list to finish
}

void Oyabun::donePopulatingFileList(const vector<FileInfo> &files)
{
    // Open the book
    _book.open(files);

    // Set the page limits
    _numPages = files.size();
    Q_ASSERT(_numPages > 0);

    // Start the pipeline
    changePages();
}

void Oyabun::turnPageForward()
{
    Q_ASSERT(_forwardEnabled);

    // Turn the page
    _book.turnPageForward();

    // Change the pages
    changePages();
}

void Oyabun::turnPageBackward()
{
    Q_ASSERT(_backwardEnabled);

    // Turn the page
    _book.turnPageBackward();

    // Change the pages
    changePages();
}

void Oyabun::turnPageForwardOnePage()
{
    Q_ASSERT(_forwardEnabled);

    // Turn the page
    _book.turnPageForwardOnePage();

    // Change the pages
    changePages();
}

void Oyabun::turnPageToStart()
{
    Q_ASSERT(_backwardEnabled);

    // Turn the page
    _book.turnPageToStart();

    // Change the pages
    changePages();
}

void Oyabun::turnPageToEnd()
{
    Q_ASSERT(_forwardEnabled);

    // Turn the page
    _book.turnPageToEnd();

    // Change the pages
    changePages();
}

void Oyabun::currentPagesReset()
{
    // Change the pages
    changePages();
}

void Oyabun::changePages()
{
    // Enable/disable navigation
    enableBackward(!_book.atStart());
    enableForward(!_book.atEnd());

    // Get the current pages
    Page *pageOne = _book.getPageOne();
    Page *pageTwo = _book.getPageTwo();

    // Depict the pages
    _depicter.depict(pageOne, pageTwo);

    // Any previously queued pages aren't needed
    _decoder.clearQueue();

    // If either page needs decoding, decode it
    if (pageOne->isDecodeNeeded()) {
        _decoder.startDecoding(pageOne);
    }
    if (pageTwo != NULL && pageTwo->isDecodeNeeded()) {
        _decoder.startDecoding(pageTwo);
    }

    // Start precaching if there's any free threads
    precache();
}

void Oyabun::decodeDone()
{
    precache();
}

void Oyabun::precache()
{
    // Pick the next page to precache
    Page *toPrecache = _book.getPageToPrecache();

    // Keep starting precaches while they're needed and there's free threads
    while (toPrecache != NULL && _decoder.isPrecacheThreadAvailable()) {

            // Start precaching
            kDebug()<<"precaching"<<toPrecache->getPageNumber();
            _decoder.startDecoding(toPrecache);

            // Pick the next page to precache
            toPrecache = _book.getPageToPrecache();
        }
}

#include "oyabun.moc"
