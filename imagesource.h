#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <QIODevice>

#include <QBuffer>
#include <QMutex>
#include <QTime>
#include <QWaitCondition>

class ImageSource : public QIODevice
{
    Q_OBJECT

public:
    ImageSource(QIODevice *proxy, int fullSize, QObject *parent = 0);
    ~ImageSource();

    qint64 bytesAvailable() const;
    bool isSequential() const;
    bool canReadLine() const;
    void close();
    bool atEnd() const;

    bool open(OpenMode mode);

    bool waitForReadyRead(int msecs);

    bool seek(qint64 pos);
    qint64 pos() const;
    qint64 size() const;

    qint64 peek(char *data, qint64 maxSize);

protected:
    qint64 readData(char *data, qint64 maxSize);
    qint64 writeData(const char *data, qint64 maxSize);

private slots:
    void proxyReadyRead();

private:
    void updateFromProxy();

private:
    static const int WAIT_TIMEOUT = 3000;

private:
    QMutex _lock;
    QWaitCondition _ready;
    QIODevice *_proxy;
    QTime _clock;
    QBuffer _buffer;
    qint64 _fullSize;
};

#endif
