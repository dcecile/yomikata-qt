#include <KDebug>

#include "decodethread.h"
#include "pagecache.h"

DecodeThread::DecodeThread(PageCache *pageCache)
    :_pageCache(pageCache), _stopped(false)
{
}

DecodeThread::~DecodeThread()
{
    // The thread had better be stopped by the time we reach here
}

void DecodeThread::stop()
{
    // Tell the thread of execution to stop
    _stopped = true;
    _decodeAborted = true;
}

void DecodeThread::abortDecode()
{
    _decodeAborted = true;
}

bool DecodeThread::decodeAborted() const
{
    return _decodeAborted;
}

PageCache *DecodeThread::pageCache()
{
    return _pageCache;
}

void DecodeThread::run()
{
    int nextPage;
    QString path;
    QSize fullSize;
    QSize boundingSize;

    kDebug()<<"Decode thread started"<<endl;

    _stopped = false;

    // Continue until stopped
    while (!_stopped) {
        _decodeAborted = false;

        // Get the next page to start decoding
        // This call will block if no pages need decoding
        _pageCache->scheduleNextDecode(this, &nextPage, &path, &fullSize, &boundingSize);

        // Check again that we haven't stopped
        if (_stopped) {
            break;
        }

        // Start decoding the page
        decode(nextPage, path, fullSize, boundingSize);
    }
}

#include "decodethread.moc"
