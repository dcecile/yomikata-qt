#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <QIODevice>

#include <QBuffer>
#include <QMutex>
#include <QTime>

class ImageSource : public QIODevice
{
    Q_OBJECT

public:
    ImageSource(QIODevice *proxy, QObject *parent = 0);
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
    qint64 writeData(const char *data, qint64 maxSize) { return 0; }

private:
    void updateFromProxy(qint64 targetSize);

private:
    QMutex _lock;
    QIODevice *_proxy;
    QTime _clock;
    QBuffer _buffer;
};

#endif
