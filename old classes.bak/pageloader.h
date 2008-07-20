#error

#ifndef PAGELOADER_H
#define PAGELOADER_H

#include <QPixmap>
#include <QImageReader>
#include <QIODevice>
#include <KJob>
#include <KUrl>
#include <threadweaver/ThreadWeaver.h>
#include <QTime>
#include <QProcess>

#include "fileinfo.h"
#include "filelister.h"
#include "extractlister.h"
#include "pagemap.h"

//using ThreadWeaver::Job; // HACK

class PageLoader : public QObject
{
    Q_OBJECT

public:
    PageLoader();

public slots:
    void setDisplaySize(const QSize &displaySize);

    void initialize(const QString &initialFile);

signals:

private:
    void startZoomMode();
    void stopZoomMode();

private slots:
    void listingDone(int initialPosition, const QStringList &files);

    //void decodeDone(Job *job);

private:
    QSize _displaySize;
    bool _resizeMode;

    PageCache _pageMap;
    int _numPages;

    //int _bufferStart;
    //int _bufferEnd;

    //ThreadWeaver::Weaver _decodeWeaver;
    //QList<DecodeJob *> _decodeBlock;
};

#endif
