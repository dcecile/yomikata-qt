#include "decoder.h"

#include <KDebug>

#include "source.h"
#include "decodethread.h"
#include "page.h"

Decoder::Decoder(Source &source, QObject *parent)
    : QObject(parent), _source(source)
{
}


Decoder::~Decoder()
{
}

void Decoder::intialize()
{
    // Delete old threads
    ThreadList::iterator i;
    for (i = _freeThreads.begin(); i != _freeThreads.end(); i++) {
        delete *i;
    }
    for (i = _busyThreads.begin(); i != _busyThreads.end(); i++) {
        delete *i;
    }
    _freeThreads.clear();
    _busyThreads.clear();

    // Set up the new threads
    for (int j = 0; j < NUM_THREADS; j++) {

        // Create new threads
        DecodeThread *temp = _source.createDecodeThread();

        // Connect to them
        connect(temp, SIGNAL(doneDecodeJob(DecodeThread *, Page *, QImage, QImage)), this, SLOT(doneDecodeJob(DecodeThread *, Page *, QImage, QImage)));
        kDebug()<<"connected";

        // Start them
        temp->start();

        // Set them as free
        _freeThreads.push_back(temp);
    }
}

void Decoder::clearQueue()
{
    // For each queued page
    while (_immediateRequests.size()) {
        // Notify that the decode was aborted
        _immediateRequests.front()->decodeAborted();
        // Remove from the queue
        _immediateRequests.pop();
    }
}

void Decoder::startDecoding(Page *page)
{
    // Notify the page that decoding has started
    page->decodeStarted();

    // If there is a free thread
    if (_freeThreads.size() > 0) {

        // Start the decode
        _freeThreads.front()->startDecoding(page);

        // Set the thread as busy
        _busyThreads.push_back(_freeThreads.front());
        _freeThreads.pop_front();

    // If all threads are busy
    } else {

        // There shouldn't be extra pages queued
        Q_ASSERT(_immediateRequests.size() < 2);

        // Queue this request
        _immediateRequests.push(page);
    }
}

bool Decoder::isPrecacheThreadAvailable()
{
    return _freeThreads.size() > 0;
}

void Decoder::doneDecodeJob(DecodeThread *thread, Page *page, QImage image, QImage thumbnail)
{
    // Set the thread as free
    ThreadList::iterator i;
    for (i = _busyThreads.begin(); i != _busyThreads.end(); i++) {
        if (*i == thread) {
            _busyThreads.erase(i);
            break;
        }
    }
    Q_ASSERT(i != _busyThreads.end());
    _freeThreads.push_back(thread);

    // Check if an immediate request is pending
    if (_immediateRequests.size() > 0) {

        // Check that the page still needs displaying

        // Start decoding the requested page
        _freeThreads.front()->startDecoding(_immediateRequests.front());

        // Set the thread as busy
        _busyThreads.push_back(_freeThreads.front());
        _freeThreads.pop_front();

        // Dequeue the request
        _immediateRequests.pop();
    }

    // Convert the image into a pixmap
    QPixmap pixmap(QPixmap::fromImage(image));
    QPixmap thumbnailPixmap(QPixmap::fromImage(thumbnail));

    // Set the thumbnail on the page
    page->setThumbnail(thumbnailPixmap);

    // Set the image on the page
    page->setPixmap(pixmap);

    // Notify Oyabun of the finished decode
    emit decodeDone();
}

#include "decoder.moc"
