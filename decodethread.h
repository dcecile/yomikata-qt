#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>

#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

class Archive;
class Indexer;
class Strategist;

class DecodeThread : public QThread
{
    Q_OBJECT

public:
    DecodeThread(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent);
    ~DecodeThread();

    void reset();

    void decode(int index);
    int currentPageNum();
    void cancel();

signals:
    void done(DecodeThread *decodeThread, int index, QImage image);
    void cancelled(DecodeThread *decodeThread);

private:
    void run();
    void setExtractCommand();
    void decode();
    void prepareForRequest();

private:
    static const int CANCELLED_POLLING = 50;
    static const int KILL_WAIT = 50;

private:
    const Archive &_archive;
    const Indexer &_indexer;
    Strategist &_strategist;

    QMutex _requestLock;
    QWaitCondition _decodeRequest;
    volatile bool _aborted;
    volatile bool _reset;
    volatile int _requestPageNum;

    volatile int _pageNum;

    volatile bool _cancelled;

    QString _command;
    QStringList _args;
};

#endif
