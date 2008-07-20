#include <KDebug>

#include "pageloader.h"
#include "filedecodejob.h"
#include "extractdecodejob.h"

const int PageLoader::BUFFER_SIZE = 12;

PageLoader::PageLoader()
{
    // Set up the decode weaver
    _decodeWeaver.setMaximumNumberOfThreads(1);
    connect(&_decodeWeaver, SIGNAL(jobDone(Job *)),
            this, SLOT(decodeDone(Job *)));

    // Connect to the listers
    connect(&_fileLister, SIGNAL(listBuilt(int, const QStringList &)),
            this, SLOT(listingDone(int, const QStringList &)));
    connect(&_extractLister, SIGNAL(listBuilt(int, const QStringList &)),
            this, SLOT(listingDone(int, const QStringList &)));

    // Start in a neutral state
    _bufferStart = -1;
    _bufferEnd = -1;

    _zoomMode = false;

    _twoPageMode = true;

    _targetPage[0] = -1;
    _targetPage[1] = -1;
    _numPages = 0;
    updateEnabledActions();
}

bool PageLoader::isPageWide(int pageNum)
{
    Q_ASSERT(pageNum < _numPages);

    if (_pages[pageNum].fullSize.isValid()) {
        // See if the page is wider than a square
        return qreal(_pages[pageNum].fullSize.width()) / qreal(_pages[pageNum].fullSize.height()) >= 1.0;
    } else {
        // Give the page the benefit of the doubt
        return false;
    }
}

void PageLoader::changePage()
{
    // Change the page to the current target page(s)

    // A page change means that zoom mode is inactive
    if (_zoomMode) {
        stopZoomMode();
    }

    if (_targetPage[1] != -1) {
        // If one of the pages is wide, only show the primary page
        if (isPageWide(_targetPage[0]) || isPageWide(_targetPage[1])) {
            kDebug()<<"Page "<<_targetPage[0]<<" or page "<<_targetPage[1]<<" is wide"<<endl;
            _targetPage[0] = _targetPage[_primaryPage];
            _targetPage[1] = -1;
        }
    }

    kDebug()<<"Changing page "<<_targetPage[0]<<", "<<_targetPage[1]<<endl;

    bool pageNumberShown = false;

    // If the pages are loaded, show them
    if (!_twoPageMode || _targetPage[1] == -1) {
        if (!_pages[_targetPage[0]].pixmap.isNull()) {
            emit showOnePage(_pages[_targetPage[0]].pixmap, _targetPage[0], _numPages);
            pageNumberShown = true;
        }
    } else {
        if (!_pages[_targetPage[0]].pixmap.isNull() && !_pages[_targetPage[1]].pixmap.isNull()) {
            emit showTwoPages(_pages[_targetPage[0]].pixmap, _pages[_targetPage[1]].pixmap, _targetPage[0], _targetPage[1], _numPages);
            pageNumberShown = true;
        }
    }

    // If a page isn't loaded or is the wrong size, decode it
    if (_pages[_targetPage[0]].pixmap.isNull() || !isPageScaled(_targetPage[0])) {
        startReadingPage(_targetPage[0]);
    }
    if (_targetPage[1] != -1 && (_pages[_targetPage[1]].pixmap.isNull() || !isPageScaled(_targetPage[1]))) {
        startReadingPage(_targetPage[1]);
    }

    // Make sure the actions are in synch
    updateEnabledActions();

    if (!pageNumberShown) {
        // Show the page number
        emit showPageNumber(_targetPage[0], _targetPage[1], _numPages);
    }

    // If we're getting near the end of the buffer, fill it up
    // Make sure that the buffer pages are decoded soon after this one
    for (int i = 1; i <= BUFFER_SIZE && i < _numPages; i++) {
        if (_targetPage[0] + i < _numPages) {
            startReadingPage(_targetPage[0] + i);
        }
        if (_targetPage[0] - i >= 0) {
            startReadingPage(_targetPage[0] - i);
        }
    }

    // Let other jobs go before the previous ones
    decodeBlockDone();
}

void PageLoader::navigateForward()
{
    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_targetPage[1] != -1) {
            _targetPage[0] += 2;
        } else {
            _targetPage[0]++;
        }

        if (_targetPage[0] + 1 < _numPages) {
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Change the page
    changePage();
}

void PageLoader::navigateBackward()
{
    Q_ASSERT(_backwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_targetPage[0] - 2 < 0) {
            _targetPage[0]--;
            _targetPage[1] = -1;

        } else {
            _targetPage[0] -= 2;
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 1;
        }

    } else {
        _targetPage[0]--;
    }

    // Change the page
    changePage();
}

void PageLoader::navigateForwardOnePage()
{
    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        _targetPage[0]++;

        if (_targetPage[0] + 1 < _numPages) {
            _targetPage[1] = _targetPage[0] + 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Change the page
    changePage();
}

void PageLoader::navigateToStart()
{
    Q_ASSERT(_backwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        _targetPage[0] = 0;

        if (_numPages > 1) {
            _targetPage[1] = 1;
            _primaryPage = 0;
        } else {
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0] = 0;
    }

    // Reset the buffer
    _bufferStart = _targetPage[0];
    _bufferEnd = _targetPage[0];

    // Change the page
    changePage();
}

void PageLoader::navigateToEnd()
{
    Q_ASSERT(_forwardEnabled);

    // Choose new target pages
    if (_twoPageMode) {
        if (_numPages > 1) {
            _targetPage[1] = _numPages - 1;
            _targetPage[0] = _numPages - 2;
            _primaryPage = 1;

        } else {
            _targetPage[0] = 0;
            _targetPage[1] = -1;
        }

    } else {
        _targetPage[0]++;
    }

    // Reset the buffer
    _bufferStart = _targetPage[0];
    _bufferEnd = _targetPage[0];

    // Change the page
    changePage();
}

bool PageLoader::isForwardEnabled() const
{
    return _forwardEnabled;
}
bool PageLoader::isBackwardEnabled() const
{
    return _backwardEnabled;
}

void PageLoader::updateEnabledActions()
{
    bool oldForward = _forwardEnabled;
    bool oldBackward = _backwardEnabled;

    // See if there's room to navigate forwards or backwards
    if (_numPages == 0) {
        _forwardEnabled = false;
        _backwardEnabled = false;
    } else {
        _backwardEnabled = _targetPage[0] > 0;
        if (_targetPage[1] == -1) {
            _forwardEnabled = _targetPage[0] + 1 < _numPages;
        } else {
            _forwardEnabled = _targetPage[1] + 1 < _numPages;
        }
    }

    // Emit the signals only of the state changed
    if (_forwardEnabled != oldForward) {
        emit forwardEnabled(_forwardEnabled);
    }
    if (_backwardEnabled != oldBackward) {
        emit backwardEnabled(_backwardEnabled);
    }
}

void PageLoader::setDisplaySize(const QSize &displaySize)
{
    //kDebug()<<"Display size "<<displaySize<<endl;
    if (_displaySize == displaySize) {
        return;
    }
    _displaySize = displaySize;

    if (_numPages > 0) {
        // A resize means that zoom mode is active
        if (!_zoomMode) {
            startZoomMode();
        }
    }
}

void PageLoader::setTwoPageMode(bool enabled)
{
    _twoPageMode = enabled;
}

void PageLoader::initialize(const QString &initialFile)
{
    // Clear everything before starting any jobs
    _pages.clear();
    _targetPage[0] = -1;
    _targetPage[1] = -1;
    _numPages = 0;
    updateEnabledActions();

    // Zoom mode doesn't start activated
    _zoomMode = false;

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
}

bool PageLoader::isPageScaled(int pageNum)
{
    Q_ASSERT(pageNum < _numPages);
    Q_ASSERT(!_pages[pageNum].pixmap.isNull());
    Q_ASSERT(_pages[pageNum].fullSize.isValid());

    QSize boundedSize(_pages[pageNum].fullSize);
    boundedSize.scale(_displaySize, Qt::KeepAspectRatio);

    //kDebug()<<"Comparing image size "<<_pages[pageNum].pixmap.size()<<"to display size "<<_displaySize<<endl;

    return _pages[pageNum].pixmap.size() == boundedSize;
}

void PageLoader::startZoomMode()
{
    Q_ASSERT(!_zoomMode);
    _zoomMode = true;

    // When switching to zoom mode, re-decode the the current pages at full resolution
    startReadingPage(_targetPage[0]);
    if (_targetPage[1] != -1) {
        startReadingPage(_targetPage[1]);
    }

    // Let other jobs go before the previous ones
    decodeBlockDone();
}

void PageLoader::stopZoomMode()
{
    Q_ASSERT(_zoomMode);
    _zoomMode = false;

    // Schedule the decoding of any unscaled pages
    _bufferStart = _targetPage[0];
    _bufferEnd = _targetPage[0];
    startReadingPage(_targetPage[0]);
    for (int i = 1; i <= BUFFER_SIZE && i < _numPages; i++) {
        if (_targetPage[0] + i < _numPages) {
            startReadingPage(_targetPage[0] + i);
        }
        if (_targetPage[0] - i >= 0) {
            startReadingPage(_targetPage[0] - i);
        }
    }

    // Let other jobs go before the previous ones
    decodeBlockDone();
}


void PageLoader::startReadingPage(int pageNum, bool highPriority)
{
    Q_ASSERT(pageNum < _pages.size());
    //Q_ASSERT(_pages[pageNum].pixmap.isNull());

    // If we're in zoom mode, decode an unscaled version of the page
    if (_zoomMode) {
        enqueuePage(pageNum, highPriority);
        // The buffer will have to be reaccessed after the zoom mode is finished

    } else {
        if (_pages[pageNum].pixmap.isNull() || !isPageScaled(pageNum)) {
            // Only reload the page if its the wrong size
            enqueuePage(pageNum, highPriority);
        }

        if (pageNum < _bufferStart) {
            _bufferStart = pageNum;
        }
        if (pageNum > _bufferEnd) {
            _bufferEnd = pageNum;
        }
    }
}

void PageLoader::enqueuePage(int pageNum, bool highPriority)
{
    const QSize &size = _zoomMode ?QSize() :_displaySize;
    // Note: if we're just resizing the window, maybe we don't need to scale from the image's
    //  full resolution. Then again, scaling from the full resolution will give the highest
    //  qualitity results

    DecodeJob *job;

    if (!_pages[pageNum].isLoading) {
        // This decode isn't queued or running, create a new job
        _pages[pageNum].isLoading = true;

        if (_archiveMode) {
            job = new ExtractDecodeJob(pageNum, _pages[pageNum].path, size, _archiveType, _archivePath, highPriority);
        } else {
            job = new FileDecodeJob(pageNum, _pages[pageNum].path, size, highPriority);
        }
        _pages[pageNum].job = job;

    } else {
        // This decode is queued or running
        job = _pages[pageNum].job;
        Q_ASSERT(job != 0);

        bool dequeued = _decodeWeaver.dequeue(job);
        if (!dequeued) {
            // The job is already running, abort starting it again
            return;
        }
        // If it's queued, move it up in the queue, giving it new parameters
        job->resetBoundingSize(size);
        job->resetPriority(highPriority);
    }

    // Start tracking how long it takes to decode the page
    _pages[pageNum].loadingTime.restart();

    // Keep track of this job so its priority can be reset later
    _decodeBlock.push_back(job);

    kDebug()<<"Queueing page "<<pageNum<<" decode, "<<size<<endl;

    _decodeWeaver.enqueue(job);
}

void PageLoader::decodeBlockDone()
{
    for (QList<DecodeJob *>::iterator i = _decodeBlock.begin(); i != _decodeBlock.end(); i++) {
        (*i)->resetPriority(false);
    }
    _decodeBlock.clear();
}

void PageLoader::decodeDone(ThreadWeaver::Job *job)
{
    DecodeJob *decodeJob = qobject_cast<DecodeJob *>(job);

    Q_ASSERT(decodeJob != 0);

    int pageNum = decodeJob->pageNum();
    Q_ASSERT(_pages[pageNum].job == decodeJob);

    // Check that the decode suceeded
    if (decodeJob->image().isNull()) {
        // Make sure the job's QImage is freed
        delete decodeJob;
        _pages[pageNum].isLoading = false;
        _pages[pageNum].job = 0;
        emit pageReadFailed(pageNum);
        return;
    }

    QTime time;
    time.start();
    _pages[pageNum].pixmap = QPixmap::fromImage(decodeJob->image());
    //kDebug()<<"QImage to QPixmap conversion: "<<time.elapsed()<<" ms"<<endl;

    kDebug()<<"Page "<<pageNum<<" decoded: "<<_pages[pageNum].loadingTime.elapsed()<<" ms - "<<_pages[pageNum].pixmap.size()<<endl;

    _pages[pageNum].fullSize = decodeJob->fullImageSize();
    _pages[pageNum].isLoading = false;

    // Make sure the job's QImage is freed
    delete decodeJob;
    _pages[pageNum].job = 0;

    // Check that the image to pixmap conversion suceeded
    if (_pages[pageNum].pixmap.isNull()) {
        emit pageReadFailed(pageNum);
        return;
    }

    // Check if the this page needs displaying
    if (!_twoPageMode || _targetPage[1] == -1) {
        if (pageNum == _targetPage[0]) {
            emit showOnePage(_pages[pageNum].pixmap, _targetPage[0], _numPages);
        }
    } else {
        // If two pages are being displayed, display them both at once
        // If one of the pages is wide, only show the primary page
        for (int i=0; i<2; i++) {
            if (pageNum == _targetPage[i]) {
                if (isPageWide(pageNum)) {
                    //kDebug()<<"Page "<<pageNum<<", target 0 wide"<<endl;
                    _targetPage[0] = _targetPage[_primaryPage];
                    _targetPage[1] = -1;

                    if (pageNum == _targetPage[0] || !_pages[_targetPage[0]].pixmap.isNull()) {
                        emit showOnePage(_pages[_targetPage[0]].pixmap, _targetPage[0], _numPages);
                    }
                    updateEnabledActions();
                } else if (!_pages[_targetPage[!i]].pixmap.isNull()) {
                    emit showTwoPages(_pages[_targetPage[0]].pixmap, _pages[_targetPage[1]].pixmap, _targetPage[0], _targetPage[1], _numPages);
                } else {
                    //kDebug()<<"Can't display "<<pageNum<<endl;
                }
                break;
            }
        }
    }

    // If the page was decoded at the wrong size, decode it over
    if ((pageNum == _targetPage[0] || pageNum == _targetPage[1]) &&
            ((_zoomMode && _pages[pageNum].pixmap.size() != _pages[pageNum].fullSize) ||
             (!_zoomMode && !isPageScaled(pageNum)))) {
        startReadingPage(pageNum);
        decodeBlockDone();
    }
}

void PageLoader::listingDone(int initialPosition, const QStringList &files)
{
    Q_ASSERT(files.size() > 0);

    Page newPage;

    // Create a page entry for each file
    kDebug()<<"Total pages: "<<files.size()<<endl;
    for (QStringList::const_iterator i = files.begin(); i != files.end(); i++) {
        newPage.path = *i;
        _pages.append(newPage);

        kDebug()<<"Path: "<<_pages.back().path<<endl;
    }
    _numPages = _pages.size();

    // Choose the starting page(s)
    _targetPage[0] = initialPosition;
    if (_twoPageMode) {
        if (_targetPage[0] + 1 < _numPages) {
            _primaryPage = 0;
            _targetPage[1] = initialPosition + 1;
        } else {
            _targetPage[1] = -1;
        }
    }

    // Synch the actions
    updateEnabledActions();

    // Start reading pages
    _bufferStart = initialPosition;
    _bufferEnd = initialPosition;
    startReadingPage(initialPosition);
    for (int i = 1; i <= BUFFER_SIZE && i < _numPages; i++) {
        if (initialPosition + i < _numPages) {
            startReadingPage(initialPosition + i);
        }
        if (initialPosition - i >= 0) {
            startReadingPage(initialPosition - i);
        }
    }

    // Let other jobs go before the previous ones
    decodeBlockDone();
}


#include "pageloader.moc"
