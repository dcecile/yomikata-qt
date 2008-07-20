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
    // Start in a neutral state
    _bufferStart = -1;
    _bufferEnd = -1;

    _resizeMode = false;

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


void PageLoader::setDisplaySize(const QSize &displaySize)
{
    //kDebug()<<"Display size "<<displaySize<<endl;
    if (_displaySize == displaySize) {
        return;
    }
    _displaySize = displaySize;

    if (_numPages > 0) {
        // A resize means that zoom mode is active
        if (!_resizeMode) {
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
    _resizeMode = false;
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
    Q_ASSERT(!_resizeMode);
    _resizeMode = true;

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
    Q_ASSERT(_resizeMode);
    _resizeMode = false;

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
    if (_resizeMode) {
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
    const QSize &size = _resizeMode ?QSize() :_displaySize;
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
}

void PageLoader::listingDone(int initialPosition, const QStringList &files)
{
}


#include "pageloader.moc"
