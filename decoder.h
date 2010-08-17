#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QObject>

#include <QFutureWatcher>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QStringList>
#include <QTemporaryFile>
#include <QTime>

class QProcess;

class Archive;
class ImageSource;
class Indexer;
class Strategist;

class Decoder : public QObject
{
    Q_OBJECT

public:
    Decoder(QObject *parent);
    ~Decoder();

    void decode(
        const Archive &archive,
        const Indexer &indexer,
        Strategist &strategist,
        int pageNum);

    int pageNum();

    void cancel();

signals:
    void done(Decoder *decoder, int pageNum, QPixmap pixmap);
    void cancelled(Decoder *decoder);

private slots:
    void decodeFinished();

private:
    QStringList chooseExtracterArguments(
        const Archive &archive,
        const QByteArray &pageFilename);
    void startExtracter(
        const Archive &archive,
        const QByteArray &pageFilename);
    void makeImageSource();
    void setUpImageReader(
        const QByteArray &pageFilename,
        Strategist &strategist);
    void startDecoding();

private:
    static const int KILL_WAIT = 50;

private:
    bool _cancelled;

    QFuture<QImage> _decodeFuture;
    QTime _time;

    int _pageNum;

    QProcess *_extracter;
    ImageSource *_imageSource;
    QImageReader _imageReader;

    QTemporaryFile _temporaryFile;
};

#endif
