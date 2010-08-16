#include "artificer.h"

#include "decodethread.h"
#include "debug.h"

Artificer::Artificer(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent)
    : QObject(parent)
{
    // Create, connect to, and start the threads
    for (int i = 0; i < DECODE_THREADS; i++)
    {
        _decodeThreads[i] = new DecodeThread(archive, indexer, strategist, this);
        connect(_decodeThreads[i],
                SIGNAL(done(DecodeThread*, int, QImage)),
                SLOT(decodeThreadDone(DecodeThread*, int, QImage)));
        connect(_decodeThreads[i], SIGNAL(cancelled(DecodeThread *)),
                SLOT(decodeThreadCancelled(DecodeThread *)));
    }

    // Nothing requested
    _request0 = -1;
    _request1 = -1;
}


Artificer::~Artificer()
{
    // Make sure the threads are stopped first
    for (int i = 0; i < DECODE_THREADS; i++)
    {
        delete _decodeThreads[i];
    }
}

void Artificer::reset()
{
    // Reset the threads
    for (int i = 0; i < DECODE_THREADS; i++)
    {
        _decodeThreads[i]->reset();
    }
}

void Artificer::decodePages(int page0, int page1)
{
    int i;
    int decoding;

    _request0 = page0;
    _request1 = page1;

    // Check current decodes
    for (i = 0; i < DECODE_THREADS; i++)
    {
        decoding = _decodeThreads[i]->currentPageNum();
        debug()<<"Thread"<<i<<"on"<<decoding;

        if (decoding != -1)
        {
            // Check if the request is already being processed
            if (decoding == _request0)
            {
                _request0 = -1;
            }
            else if (decoding == _request1)
            {
                _request1 = -1;
            }
            // Stop the thread if it's not doing the request -- current request should
            //   perform fast
            else
            {
                _decodeThreads[i]->cancel();
            }
        }
    }

    // Assign a request to a free thread
    for (i = 0; i < DECODE_THREADS; i++)
    {
        if (_decodeThreads[i]->currentPageNum() == -1)
        {
            if (_request0 >= 0)
            {
                _decodeThreads[i]->decode(_request0);
                _request0 = -1;
            }
            else if (_request1 >= 0)
            {
                _decodeThreads[i]->decode(_request1);
                _request1 = -1;
            }
        }
    }

    debug()<<"Queued"<<_request0<<_request1;
}

/**
 * @todo Don't convert to pixmap for unneeded pages.
 * @todo Handle case where signal comes back and decode is already requested
 */
void Artificer::decodeThreadDone(DecodeThread *decodeThread, int index, QImage image)
{
    // Start a new decode if needed
    if (decodeThread->currentPageNum() == -1)
    {
        if (_request0 >= 0)
        {
            decodeThread->decode(_request0);
            _request0 = -1;
        }
        else if (_request1 >= 0)
        {
            decodeThread->decode(_request1);
            _request1 = -1;
        }
    }

    // Conver the image to a pixmap
    QPixmap pixmap(QPixmap::fromImage(image));

    // Notify the steward
    emit pageDecoded(index, pixmap);
}

void Artificer::decodeThreadCancelled(DecodeThread *decodeThread)
{
    // Cancelled
    debug()<<"Cancelled"<<decodeThread->currentPageNum();

    // Start a new decode if needed
    if (decodeThread->currentPageNum() == -1)
    {
        if (_request0 >= 0)
        {
            debug()<<"Starting"<<_request0;
            decodeThread->decode(_request0);
            _request0 = -1;
        }
        else if (_request1 >= 0)
        {
            debug()<<"Starting"<<_request1;
            decodeThread->decode(_request1);
            _request1 = -1;
        }
    }
}

#include "artificer.moc"
