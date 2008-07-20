#include <kio/jobclasses.h>
#include <kio/scheduler.h>
#include <kio/jobuidelegate.h>
#include <KDebug>

#include "pageloader.h"
#include "filetypes.h"
#include "filedecodejob.h"

PageLoader::PageLoader()
{
    _decodeWeaver.setMaximumNumberOfThreads(1);
    connect(&_decodeWeaver, SIGNAL(jobDone(Job *)),
            this, SLOT(decodeDone(Job *)));
}

void PageLoader::initialize(const QString &initialFile)
{
    // Clear everything before starting any jobs
    _pages.clear();


    // Start decoding the image
    KUrl path = KUrl::fromPath(initialFile);
    _pages.append(Page());
    _pages.front().path = path;

    _pages.front().pixmap = QPixmap(initialFile);
    if (_pages.front().pixmap.isNull()) {
        emit pageReadFailed(0);
    } else {;
        emit pageRead(0);
    }


    // Start listing the directory

    // Find the directory of the initial file
    _currentDir = path.directory();

    if (_currentDir == KUrl()) {
        // Directory operation failed
        kDebug()<<"Unable to get directory of "<<KUrl::fromPath(initialFile)<<endl;
        return;
    }

    // Create the list job and hook up to its signals
    _listJob = KIO::listDir(_currentDir);
    connect(_listJob, SIGNAL(result(KJob *)),
            this, SLOT(listingFinished(KJob *)));
    connect(_listJob, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)),
            this, SLOT(listingEntries(KIO::Job *, const KIO::UDSEntryList &)));

    // Start the listing
    KIO::Scheduler::scheduleJob(_listJob);
}

void PageLoader::listingEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
    Q_ASSERT(job == _listJob);

    //kDebug()<<"Progress: "<<_listJob->percent()<<"%"<<endl<<"Entries: "<<list.size()<<endl;

    Page tempPage;

    // Some directory residents have been found; go through the list looking for images
    for (KIO::UDSEntryList::const_iterator i = list.begin(); i != list.end(); i++) {
        if (!i->isDir()) {
            tempPage.path = _currentDir;
            tempPage.path.addPath(i->stringValue(KIO::UDS_NAME));

            if (FileTypes::determineType(tempPage.path) == Image) {
                // This file is an image, add it to the list of pages
                _pages.append(tempPage);
            }
        }
    }
}

void PageLoader::listingFinished(KJob *job)
{
    Q_ASSERT(job == _listJob);

    int initialPosition;

    kDebug()<<"Job finished."<<endl;

    if (_listJob->error()) {
        // There was an error getting the directory listing
        // Only let the user view the one file
        while (_pages.size() > 1) {
            _pages.pop_back();
        }
        _listJob->ui()->showErrorMessage();

        initialPosition = 0;

    } else {
        // Remove the initial file from the list after determining its position
        KUrl initialPath = _pages.front().path;
        initialPosition = 0;
        for (QList<Page>::iterator i = _pages.begin() + 1; i != _pages.end(); i++) {
            if (i->path == initialPath) {
                // The initial page has been found
                // HACK copy the pixmap
                i->pixmap = _pages.front().pixmap;
                break;
            }
            initialPosition++;
        }

        // Remove the duplicate from the list of pages
        _pages.pop_front();

        Q_ASSERT(initialPosition < _pages.size());
    }

    kDebug()<<"Total pages: "<<_pages.size()<<endl;
    for (QList<Page>::iterator i = _pages.begin(); i != _pages.end(); i++) {
        kDebug()<<"Path: "<<i->path.url()<<endl;
    }
    emit pagesListed(0, _pages.size());

    // Start listing the parent directory
    KUrl parentDir = _currentDir;
    if (!parentDir.cd("..")) {
        kDebug()<<"Can't go up one directory from "<<_currentDir.prettyUrl()<<endl;
    } else {
        kDebug()<<"Parent dir: "<<parentDir<<endl;;
    }
}

int PageLoader::numPages() const
{
    return _pages.size();
}

bool PageLoader::isPageRead(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());

    return _pages[pageNum].pixmap.isNull();
}

void PageLoader::startReadingPage(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());
    Q_ASSERT(_pages[pageNum].pixmap.isNull());
    Q_ASSERT(!_pages[pageNum].isLoading);

    _pages[pageNum].isLoading = true;

    _decodeWeaver.enqueue(new FileDecodeJob(pageNum, _pages[pageNum].path));
}

void PageLoader::decodeDone(ThreadWeaver::Job *job)
{
    FileDecodeJob *decodeJob = dynamic_cast<FileDecodeJob *>(job);

    kDebug()<<"Decode job done"<<endl;

    _pages[decodeJob->pageNum()].pixmap = decodeJob->pixmap();

    if (_pages[decodeJob->pageNum()].pixmap.isNull()) {
        emit pageReadFailed(decodeJob->pageNum());
    } else {;
        emit pageRead(decodeJob->pageNum());
    }
}

QPixmap PageLoader::getPage(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());
    return _pages[pageNum].pixmap;
}

#include "pageloader.moc"
