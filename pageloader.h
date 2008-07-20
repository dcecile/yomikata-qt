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
#include "decodejob.h"

using ThreadWeaver::Job; // HACK

class PageLoader : public QObject
{
    Q_OBJECT

public:
    PageLoader();

    bool isForwardEnabled() const;
    bool isBackwardEnabled() const;

public slots:
    void setDisplaySize(const QSize &displaySize);
    void setTwoPageMode(bool enabled);

    void initialize(const QString &initialFile);

    void navigateForward();
    void navigateBackward();
    void navigateForwardOnePage();
    void navigateToStart();
    void navigateToEnd();

signals:
    void forwardEnabled(bool enabled);
    void backwardEnabled(bool enabled);

    void showOnePage(const QPixmap &page, int pageNum, int totalPages);
    void showTwoPages(const QPixmap &pageA, const QPixmap &pageB, int pageNumA, int pageNumB, int totalPages);

    void showPageNumber(int pageNumA, int pageNumB, int totalPages);

    void pageReadFailed(int pageNum);

private:
    void updateEnabledActions();

    void startReadingPage(int pageNum, bool highPriority = true);
    void enqueuePage(int pageNum, bool highPriority);
    void decodeBlockDone();

    void changePage();

    bool isPageScaled(int pageNum);
    bool isPageWide(int pageNum);

    void startZoomMode();
    void stopZoomMode();

private slots:
    void listingDone(int initialPosition, const QStringList &files);

    void decodeDone(Job *job);

private:
    bool _twoPageMode;

    int _primaryPage;
    int _targetPage[2];

    QSize _displaySize;
    bool _zoomMode;
    bool _forwardEnabled;
    bool _backwardEnabled;

    bool _archiveMode;

    struct Page
    {
        Page() :isLoading(false), isScaled(false) {}
        QString path;
        QPixmap pixmap;
        QSize fullSize;
        bool isLoading;
        bool isScaled;
        QTime loadingTime;
        DecodeJob *job;
    };

    QList<Page> _pages;
    int _numPages;

    int _bufferStart;
    int _bufferEnd;

    ThreadWeaver::Weaver _decodeWeaver;
    QList<DecodeJob *> _decodeBlock;

    FileLister _fileLister;
    ExtractLister _extractLister;

    QString _archivePath;
    FileInfo::ArchiveType _archiveType;

private:
    static const int BUFFER_SIZE;
};

#endif
