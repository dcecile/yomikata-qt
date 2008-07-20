#ifndef PAGELOADER_H
#define PAGELOADER_H

#include <QPixmap>
#include <QImageReader>
#include <QIODevice>
#include <KJob>
#include <KUrl>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <threadweaver/ThreadWeaver.h>
#include <QTime>
#include <QProcess>

using namespace ThreadWeaver; // HACK

class PageLoader : public QObject
{
    Q_OBJECT

public:
    PageLoader();

    void setDisplaySize(const QSize &displaySize);

    void initialize(const QString &initialFile);

    int numPages() const;

    bool isPageRead(int pageNum);

    bool isPageScaled(int pageNum);

    void startZoomMode();
    void stopZoomMode(int nextPage);
    bool isZoomMode() const;

    void startReadingPage(int pageNum);

    const QPixmap &getPage(int pageNum);

signals:
    void pagesListed(int initialPosition, int numPages);
    void pageRead(int pageNum);
    void pageReadFailed(int pageNum);

private slots:
    void decodeDone(Job *job);

    void listingEntries(KIO::Job *job, const KIO::UDSEntryList &list);
    void listingFinished(KJob *job);

    void extracterOutputText();
    void extracterErrorText();
    void extracterError(QProcess::ProcessError error);
    void extracterFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool _archiveMode;

    QSize _displaySize;
    bool _zoomMode;

    KUrl _currentDir;
    KIO::ListJob *_listJob;

    struct Page
    {
        Page() :isLoading(false), isScaled(false) {}
        QString path;
        QPixmap pixmap;
        QSize fullSize;
        bool isLoading;
        bool isScaled;
        QTime loadingTime;

        int fileSize;
        int compressedSize;
    };

    QList<Page> _pages;

    int _bufferStart;
    int _bufferEnd;

    ThreadWeaver::Weaver _decodeWeaver;

    QString _archivePath;
    QProcess _extracterProcess;
    bool _listingBodyReached;
    bool _listingBodyFinished;
    bool _filenameLine;
    QByteArray _currentInputLine;

private:
    static const int BUFFER_SIZE;
};

#endif
