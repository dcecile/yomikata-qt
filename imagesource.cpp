#include "imagesource.h"

#include "debug.h"

ImageSource::ImageSource(QIODevice *proxy, QObject *parent)
    : QIODevice(parent)
{
    _proxy = proxy;
    //qDebug()<<"Open"<<_proxy->isOpen();
    setOpenMode(ReadOnly);
    _clock.start();
}


ImageSource::~ImageSource()
{
}

bool ImageSource::waitForReadyRead(int msecs)
{
    debug()<<"Waiting";
    return false;
}

bool ImageSource::open(OpenMode mode)
{
    debug()<<"Mode"<<mode;
    return false;
}

qint64 ImageSource::bytesAvailable() const
{
    debug()<<"Available";
    return 0;
}

bool ImageSource::isSequential() const
{
    //debug()<<"isSequential";
    return true;
}

bool ImageSource::canReadLine() const
{
    debug()<<"canReadLine";
    return false;
}

void ImageSource::close()
{
    debug()<<"close";
}

bool ImageSource::atEnd() const
{
    debug()<<"atEnd";
    return false;
}

qint64 ImageSource::readData(char *data, qint64 maxSize)
{
    int read = _proxy->read(data, maxSize);
    //debug()<<"Reading"<<read<<QString("(%1 ms)").arg(_clock.elapsed());
    return read;
}

#include "imagesource.moc"
