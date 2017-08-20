#include "artificer.h"

#include <QThread>
#include <QThreadPool>

#include "debug.h"
#include "decoder.h"

Artificer::Artificer(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent)
    : QObject(parent), _archive(archive), _indexer(indexer), _strategist(strategist)
{
    // Set up thread pool
    QThreadPool::globalInstance()->setMaxThreadCount(DECODE_THREADS);
    //debug()<<"Ideal threads:"<<QThread::idealThreadCount();
}


Artificer::~Artificer()
{
}

void Artificer::reset()
{
    foreach (Decoder *decoder, _running)
    {
        delete decoder;
    }
    _running.clear();

    foreach (Decoder *decoder, _cancelled)
    {
        delete decoder;
    }
    _cancelled.clear();
}

void Artificer::decodePages(int page0, int page1)
{
    QList<int> pages;

    if (page0 != -1)
    {
        pages<<page0;
    }

    if (page1 != -1)
    {
        pages<<page1;
    }

    decodePages(pages);
}

    
void Artificer::decodePages(QList<int> pages)
{
    // See which decoders need cancelling
    foreach (Decoder *decoder, _running)
    {
        int foundRequest = pages.indexOf(decoder->pageNum());

        if (foundRequest >= 0)
        {
            pages.removeAt(foundRequest);
        }
        else
        {
            decoder->cancel();
            _running.removeOne(decoder);
            _cancelled<<decoder;
        }
    }

    // Queue the new pages if needed
    // TODO: Reduce overload when changing pages rapidly, spawning
    //   processes without limit
    foreach (int request, pages)
    {
        // Create the decoder
        Decoder *decoder = new Decoder(this);
        connect(decoder,
            SIGNAL(done(Decoder*, int, QPixmap)),
            SLOT(decoderDone(Decoder*, int, QPixmap)));
        connect(decoder,
            SIGNAL(cancelled(Decoder *)),
            SLOT(decoderCancelled(Decoder *)));
        _running<<decoder;

        // Start it
        decoder->decode(_archive, _indexer, _strategist, request);
    }
}

void Artificer::decoderDone(Decoder *decoder, int index, QPixmap pixmap)
{
    // Delete the decoder
    bool removed = _running.removeOne(decoder);
    delete decoder;
    Q_ASSERT(removed);

    // Notify the steward
    emit pageDecoded(index, pixmap);
}

void Artificer::decoderCancelled(Decoder *decoder)
{
    // Delete the decoder
    bool removed = _cancelled.removeOne(decoder);
    delete decoder;
    Q_ASSERT(removed);
}
