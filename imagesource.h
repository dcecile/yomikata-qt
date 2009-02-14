#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <QIODevice>
#include <QTime>
#include <QMutex>

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

protected:
    qint64 readData(char *data, qint64 maxSize);
    qint64 writeData(const char *data, qint64 maxSize) { return 0; }

private:
    QMutex _lock;
    QIODevice *_proxy;
    QTime _clock;
};

#endif
