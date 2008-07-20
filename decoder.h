#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QPixmap>
#include <QImage>

#include <list>
#include <queue>

using std::list;
using std::queue;

class Source;
class Page;
class DecodeThread;

/**
 * @defgroup mod_decoder Decoder module
 *
 */

/**
 * @ingroup mod_decoder
 * @brief Manage page decoding
 * @todo take page position data
 */
class Decoder : public QObject
{
    Q_OBJECT

public:
    Decoder(Source &source, QObject *parent);

    void intialize();

    void clearQueue();
    void startDecoding(Page *page);

    bool isPrecacheThreadAvailable();

    ~Decoder();

signals:
    void decodeDone();

private slots:
    void doneDecodeJob(DecodeThread *thread, Page *page, QImage image, QImage thumbnail);


private:
    static const int NUM_THREADS = 2;

private:
    Source &_source;
    typedef list<DecodeThread *> ThreadList;
    ThreadList _freeThreads;
    ThreadList _busyThreads;

    queue<Page *> _immediateRequests;
};

#endif
