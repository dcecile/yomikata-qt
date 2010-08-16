#include "imagesource.h"

#include "debug.h"

ImageSource::ImageSource(QIODevice *proxy, QObject *parent)
    : QIODevice(parent)
{
    _proxy = proxy;
    Q_ASSERT(_proxy->isReadable());
    setOpenMode(ReadOnly);
    _buffer.setData(_proxy->readAll());
    _buffer.open(ReadOnly);
    _clock.start();
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
    QMutexLocker locker(&_lock);
    //debug()<<"close";
    _proxy->close();
    _buffer.close();
    QIODevice::close();
}

void ImageSource::updateFromProxy(qint64 targetSize)
{
    bool waited = true;

    //debug()<<targetSize<<_buffer.size();

    while (waited && (targetSize - _buffer.size() > 0))
    {
        waited = _proxy->waitForReadyRead(-1);
        _buffer.buffer() += _proxy->readAll();
    }
}

qint64 ImageSource::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&_lock);
    updateFromProxy(pos() + maxSize);
    int read = _buffer.read(data, maxSize);
    //debug()<<"Reading"<<read<<QString("(%1 ms)").arg(_clock.elapsed())<<isOpen();
    return read;
}

bool ImageSource::seek(qint64 pos)
{
    QMutexLocker locker(&_lock);

    if (isReadable())
    {
        QIODevice::seek(pos);
        updateFromProxy(pos + 1);
        return _buffer.seek(pos);
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

qint64 ImageSource::size() const
{
    Q_ASSERT(false);
    return 0;
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

bool ImageSource::open(OpenMode mode)
{
    Q_ASSERT(false);
    return false;
}

#include "imagesource.moc"
