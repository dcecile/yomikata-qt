#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <QSize>

class PageCache;

class DecodeThread : public QThread
{
    Q_OBJECT

public:
    DecodeThread(PageCache *pageCache);
    virtual ~DecodeThread();

    void abortDecode();

    void stop();

signals:
    void pageReadFailed(int pageNum);

protected:
    virtual void decode(int pageNum, const QString &path, QSize fullSize, const QSize &boundingSize) = 0;

    PageCache *pageCache();

    bool decodeAborted() const;

private:
    void run();

private:
    PageCache *_pageCache;
    bool _stopped;
    bool _decodeAborted;
};

#endif
