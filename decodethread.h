#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QObject>

#include <QImage>
#include <QTime>
#include <QStringList>
#include <QFutureWatcher>
#include <QImageReader>

class QProcess;

class Archive;
class Indexer;
class Strategist;
class ImageSource;

class DecodeThread : public QObject
{
    Q_OBJECT

public:
    DecodeThread(const Archive &archive, const Indexer &indexer, Strategist &strategist, QObject *parent);
    ~DecodeThread();

    void reset();

    void decode(int index);
    int currentPageNum();
    void cancel();

signals:
    void done(DecodeThread *decodeThread, int index, QImage image);
    void cancelled(DecodeThread *decodeThread);

private slots:
    void decodeFinished();

private:
    void setExtractCommand();
    void decode();

private:
    const Archive &_archive;
    const Indexer &_indexer;
    Strategist &_strategist;

    bool _aborted;
    bool _reset;
    int _requestPageNum;
    bool _cancelled;

    QImageReader _imageReader;
    QFutureWatcher<QImage> _decodeWatcher;
    QFuture<QImage> _decodeFuture;
    QTime _time;

    int _pageNum;

    QProcess *_extracter;
    ImageSource *_imageSource;

    QString _command;
    QStringList _args;
    QString _temporaryFileName;
};

#endif
