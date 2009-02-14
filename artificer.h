#ifndef ARTIFICER_H
#define ARTIFICER_H

#include <QObject>

#include <QPixmap>

class Archive;
class Indexer;
class Strategist;
class DecodeThread;

class Artificer : public QObject
{
    Q_OBJECT

public:
    Artificer(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent = NULL);
    ~Artificer();

    void reset();

    void decodePages(int page0, int page1);

signals:
    void pageDecoded(int index, QPixmap pixmap);

private slots:
    void decodeThreadDone(DecodeThread *decodeThread, int index, QImage image);
    void decodeThreadCancelled(DecodeThread *decodeThread);

private:
    static const int DECODE_THREADS = 2;

private:
    DecodeThread *_decodeThreads[DECODE_THREADS];
    int _request0;
    int _request1;
};

#endif
