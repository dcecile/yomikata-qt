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

using namespace ThreadWeaver; // HACK

class PageLoader : public QObject
{
    Q_OBJECT

public:
    PageLoader();

    void initialize(const QString &initialFile);

    int numPages() const;

    bool isPageRead(int pageNum);

    void startReadingPage(int pageNum);

    QPixmap getPage(int pageNum);

signals:
    void pagesListed(int initialPosition, int numPages);
    void pageRead(int pageNum);
    void pageReadFailed(int pageNum);

private slots:
    void listingEntries(KIO::Job *job, const KIO::UDSEntryList &list);
    void listingFinished(KJob *job);

    void decodeDone(Job *job);

private:
    KUrl _currentDir;
    KIO::ListJob *_listJob;

    struct Page
    {
        Page() :isLoading(false) {}
        KUrl path;
        QPixmap pixmap;
        bool isLoading;
    };

    QList<Page> _pages;

    ThreadWeaver::Weaver _decodeWeaver;
};

#endif
