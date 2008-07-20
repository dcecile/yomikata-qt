#include "decodethread.h"

#include <QMutexLocker>
#include <KDebug>

#include "page.h"

DecodeThread::DecodeThread(QObject *parent)
    : QThread(parent), _finished(true), _aborted(false)
{
}

DecodeThread::~DecodeThread()
{
    // Abort; make sure the thread stops running
    _aborted = true;
    _waitForDecode.wakeOne();
    wait();
}

void DecodeThread::run()
{
    QMutexLocker locker(&_mutex);

    kDebug()<<"Started";

    // Wait for the first decode
    _waitForDecode.wait(&_mutex);

    // Stop if aborted
    while (!_aborted) {

        // Decode
        _finished = false;
        decode();

        // Wait for the next decode
        _waitForDecode.wait(&_mutex);
    }

    kDebug()<<"Aborted";
}

void DecodeThread::startDecoding(Page *page)
{
    // Assert the decode is finished so that it will be or will soon start waiting
    Q_ASSERT(_finished);

    // Make sure the thread is waiting
    _mutex.lock();

    // Set the parameters
    _page = page;

    // Trigger the decode
    _waitForDecode.wakeOne();

    // Unlock the thread
    _mutex.unlock();
}

void DecodeThread::finished(QImage image, QImage thumbnail)
{
    _finished = true;
    emit doneDecodeJob(this, _page, image, thumbnail);
}

#include "decodethread.moc"
