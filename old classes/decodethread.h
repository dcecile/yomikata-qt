#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

class Page;

/**
 * @ingroup mod_decoder
 */
class DecodeThread : public QThread
{
    Q_OBJECT

protected:
    DecodeThread(QObject *parent);

public:
    void startDecoding(Page *page);

    virtual ~DecodeThread();

signals:
    void doneDecodeJob(DecodeThread *thread, Page *page, QImage image, QImage thumbnail);

protected:
    virtual void decode() = 0;
    void finished(QImage image, QImage thumbnail);

private:
    void run();

private:
    QMutex _mutex;
    QWaitCondition _waitForDecode;
    bool _finished;
    bool _aborted;

protected:
    Page *_page;
};

#endif
