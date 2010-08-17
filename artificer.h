#ifndef ARTIFICER_H
#define ARTIFICER_H

#include <QObject>

#include <QPixmap>

class Archive;
class Decoder;
class Indexer;
class Strategist;

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
    void decoderDone(Decoder *decoder, int index, QPixmap pixmap);
    void decoderCancelled(Decoder *decoder);

private:
    void decodePages(QList<int> pages);

private:
    static const int DECODE_THREADS = 8;

private:
    const Archive &_archive;
    const Indexer &_indexer;
    Strategist &_strategist;

    QList<Decoder *> _running;
    QList<Decoder *> _cancelled;
};

#endif
