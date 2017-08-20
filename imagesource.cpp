#include "imagesource.h"

#include <QProcess>

#include "debug.h"

// QProcess used from a non-owner thread, calling waitFoReadyRead is
// very unreliable (causes exit 141, SIGPIPE). So, the data is pulled
// from the proxy using signals, and an off-thread wait condition.
ImageSource::ImageSource(QIODevice *proxy, int fullSize, QObject *parent)
    : QIODevice(parent)
{
    _proxy = proxy;
    Q_ASSERT(_proxy->isReadable());
    setOpenMode(ReadOnly | Unbuffered);
    _buffer.setData(_proxy->readAll());
    _buffer.open(ReadOnly);
    _fullSize = static_cast<qint64>(fullSize);
    _clock.start();
    connect(_proxy, SIGNAL(readyRead()), SLOT(proxyReadyRead()));
}

ImageSource::~ImageSource()
{
}

bool ImageSource::isSequential() const
{
    return false;
}

void ImageSource::close()
{
    // Close this buffer and prevent new references (rely
    // on the proxy to get closed by other means)
    QMutexLocker locker(&_lock);
    _buffer.close();
    QIODevice::close();
    _ready.wakeOne();
}

void ImageSource::proxyReadyRead()
{
    QMutexLocker locker(&_lock);
    if (isReadable())
    {
        _buffer.buffer() += _proxy->readAll();
    }
    _ready.wakeOne();
}

void ImageSource::updateFromProxy()
{
    qint64 targetSize = qMin(pos() + 1, _fullSize);
    bool problemWaiting = false;

    while (!problemWaiting && _buffer.size() < targetSize)
    {
        problemWaiting =
            !_ready.wait(&_lock, WAIT_TIMEOUT)
            || !isReadable();
    }

    if (problemWaiting && _buffer.size() < targetSize && isReadable())
    {
        QProcess *process = (QProcess *) _proxy;
        debug()<<"Problem!"<<process->exitCode()<<process->error()<<process->state()<<process->isReadable();
    }
}

qint64 ImageSource::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&_lock);
    if (isReadable())
    {
        updateFromProxy();
        if (isReadable())
        {
            return _buffer.read(data, maxSize);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

bool ImageSource::seek(qint64 pos)
{
    QMutexLocker locker(&_lock);

    if (isReadable())
    {
        updateFromProxy();
        if (isReadable())
        {
            QIODevice::seek(pos);
            return _buffer.seek(pos);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

qint64 ImageSource::pos() const
{
    return _buffer.pos();
}

bool ImageSource::open(OpenMode mode)
{
    return mode == ReadOnly;
}

qint64 ImageSource::size() const
{
    return _fullSize;
}

qint64 ImageSource::peek(char *data, qint64 maxSize)
{
    Q_ASSERT(false);
    return 0;
}

bool ImageSource::waitForReadyRead(int msecs)
{
    Q_ASSERT(false);
    return false;
}

bool ImageSource::canReadLine() const
{
    Q_ASSERT(false);
    return false;
}

bool ImageSource::atEnd() const
{
    Q_ASSERT(false);
    return false;
}

qint64 ImageSource::bytesAvailable() const
{
    Q_ASSERT(false);
    return 0;
}

qint64 ImageSource::writeData(const char *data, qint64 maxSize)
{
    Q_ASSERT(false);
    return 0;
}
