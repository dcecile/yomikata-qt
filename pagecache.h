#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QImage>
#include <QEvent>
#include <QTime>

#include <set>
#include <list>
#include <queue>

using std::set;
using std::list;
using std::queue;

class DecodeThread;

class PageCache : public QObject
{
    Q_OBJECT

public:
    PageCache(bool twoPageMode);

    void setData(int pageNum, const QImage &image, const QSize &fullSize);

    void scheduleNextDecode(DecodeThread *decodingThread, int *nextPageNum, QString *path, QSize *fullSize, QSize *boundingSize);

    bool event(QEvent *event);

public slots:
    void reset();
    void initialize(QStringList files, QString startPageName);

    void setTwoPageMode(bool enabled);

    void setDisplaySize(const QSize &displaySize);

public slots:
    void navigateForward();
    void navigateBackward();
    void navigateForwardOnePage();
    void navigateToStart();
    void navigateToEnd();

signals:
    void pageDecode(int pageNum);

    void showOnePage(const QPixmap &page, int pageNum, int totalPages);
    void showTwoPages(const QPixmap &pageA, const QPixmap &pageB, int pageNumA, int pageNumB, int totalPages);

    void showPageNumber(int pageNumA, int pageNumB, int totalPages);

    void forwardEnabled(bool enabled);
    void backwardEnabled(bool enabled);

private:
    void printCached();

    int findNextBestPage();

    void pageDecoded(int pageNum);
    void deallocatePage(int pageNum);

    void requestPage(int pageNum);

    void updateEnabledActions();
    void changePage();

    int memoryUse(int pageNum);

private:
    struct Page
    {
        Page() :decodingThread(0), isScaled(false), isRequested(false), isWide(false) {}
        QString path;
        QPixmap pixmap;
        QImage image;
        QSize fullSize;
        DecodeThread *decodingThread;
        QSize decodingBoundingSize;
        bool isScaled;
        bool isRequested;
        bool isWide;
        QTime loadingTime;
    };

private:
    QMutex _mutex;

    QList<Page> _pages;
    int _numPages;

    int _primaryPage;
    int _targetPage[2];
    int _targetRequested[2];

    QWaitCondition _noRequestedPages;

    set<int> _pagesRequested;
    set<int>::iterator _requestedPosition;
    set<int> _pagesCached;
    int _cachedSize;

    int _bufferStart;
    int _bufferEnd;

    bool _twoPageMode;
    QSize _displaySize;
    bool _resizeMode;

    set<int> _decodedPages;

    set<int> _pagesToDeallocate;

    bool _forwardEnabled;
    bool _backwardEnabled;

private:
    static const int CACHE_LIMIT;
    static const QEvent::Type DECODED_EVENT;
    static const QEvent::Type DEALLOCATE_EVENT;
};

#endif
