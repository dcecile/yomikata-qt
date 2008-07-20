#include <kio/scheduler.h>
#include <kio/jobuidelegate.h>

#include "filelister.h"
#include "fileinfo.h"

FileLister::FileLister()
{
    _listJob = 0;
}

void FileLister::list(const QString &initialFile)
{
    Q_ASSERT(_listJob == 0);

    // Clear the list
    _fileList.clear();

    // Start the timer
    _listingTime.restart();

    // Remember the first image
    _initialFile = initialFile;

    // Start listing the directory
    // Find the directory of the initial file
    KUrl test = KUrl::fromPath(initialFile);
    kDebug()<<"KUrl "<<test.url()<<endl;
    _currentDir = KUrl::fromPath(initialFile).directory();

    Q_ASSERT(!_currentDir.isEmpty());

    // Create the list job and hook up to its signals
    _listJob = KIO::listDir(_currentDir, false);
    connect(_listJob, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)),
            this, SLOT(entries(KIO::Job *, const KIO::UDSEntryList &)));
    connect(_listJob, SIGNAL(result(KJob *)),
            this, SLOT(finished(KJob *)));

    // Start the listing
    KIO::Scheduler::scheduleJob(_listJob);

    kDebug()<<"Started directory listing for "<<initialFile<<endl;
}

void FileLister::entries(KIO::Job *job, const KIO::UDSEntryList &list)
{
    Q_ASSERT(job == _listJob);

    //kDebug()<<"Progress: "<<_listJob->percent()<<"%"<<endl<<"Entries: "<<list.size()<<endl;

    KUrl tempPath;
    QString tempString;

    // Some directory residents have been found; go through the list looking for images
    for (KIO::UDSEntryList::const_iterator i = list.begin(); i != list.end(); i++) {
        if (!i->isDir()) {
            tempPath = _currentDir;
            tempPath.addPath(i->stringValue(KIO::UDS_NAME));
            tempString = tempPath.toLocalFile();

            Q_ASSERT(!tempString.isEmpty());

            if (FileInfo::isImageFile(tempString)) {
                // This file is an image, add it to the list of pages
                _fileList<<tempString;
            }
        }
    }
}

void FileLister::finished(KJob *job)
{
    Q_ASSERT(job == _listJob);

    kDebug()<<"Listing finished: "<<_listingTime.elapsed()<<" ms"<<endl;

    if (_listJob->error()) {
        // There was an error getting the directory listing
        // FIXME: Pop up an error message
        _listJob->ui()->showErrorMessage();

        // Only let the user view the one file
        emit listBuilt(QStringList(_initialFile), _initialFile);

    } else {
        // The listing is done
        emit listBuilt(_fileList, _initialFile);
    }

    _listJob = 0;

    /*
    // Start listing the parent directory
    KUrl parentDir = _currentDir;
    if (!parentDir.cd("..")) {
        kDebug()<<"Can't go up one directory from "<<_currentDir.prettyUrl()<<endl;
    } else {
        kDebug()<<"Parent dir: "<<parentDir<<endl;
    }*/
}

#include "filelister.moc"
