#include <kio/jobclasses.h>
#include <kio/scheduler.h>
#include <kio/jobuidelegate.h>
#include <KDebug>
#include <QDir>

#include "pageloader.h"
#include "filetypes.h"
#include "decodejob.h"
#include "filedecodejob.h"
#include "extractdecodejob.h"

const int PageLoader::BUFFER_SIZE = 20;

PageLoader::PageLoader()
{
    // Set up the decode weaver
    _decodeWeaver.setMaximumNumberOfThreads(1);
    connect(&_decodeWeaver, SIGNAL(jobDone(Job *)),
            this, SLOT(decodeDone(Job *)));

    // Set up the decompressor process
    connect(&_extracterProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(extracterOutputText()));
    connect(&_extracterProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(extracterErrorText()));
    connect(&_extracterProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(extracterError(QProcess::ProcessError)));
    connect(&_extracterProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(extracterFinished(int, QProcess::ExitStatus)));

    // Start in a neutral state
    _bufferStart = -1;
    _bufferEnd = -1;

    _zoomMode = false;
}

void PageLoader::setDisplaySize(const QSize &displaySize)
{
    _displaySize = displaySize;
}

void PageLoader::initialize(const QString &initialFile)
{
    // Clear everything before starting any jobs
    _pages.clear();

    // Zoom mode doesn't start activated
    _zoomMode = false;

    KUrl path = KUrl::fromPath(initialFile);

    // Check if we're opening plain files or an archive
    FileType fileType = FileTypes::determineType(initialFile);
    Q_ASSERT(fileType == Image || fileType == Archive);

    if (fileType == Image) {
        _archiveMode = false;

        // Remember the first image
        _pages.append(Page());
        _pages.front().path = initialFile;

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

        kDebug()<<"Started directory listing for "<<initialFile<<endl;

    } else {
        _archiveMode = true;

        _archivePath = initialFile;
        _currentDir = path.directory();

        // Check that the temp directory is writable
        //QDir::tempPath();

        _listingBodyReached = false;
        _filenameLine = true;
        _listingBodyFinished = false;
        _currentInputLine = "";

        // List the contents of the archive
        QStringList args;
        args.append("vr");
        args.append(_archivePath);
        _extracterProcess.start("unrar", args);

        kDebug()<<"Started archive listing for "<<initialFile<<endl;
    }
}


int PageLoader::numPages() const
{
    return _pages.size();
}

bool PageLoader::isPageRead(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());

    return !_pages[pageNum].pixmap.isNull();
}

bool PageLoader::isPageScaled(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());
    Q_ASSERT(!_pages[pageNum].pixmap.isNull());
    Q_ASSERT(_pages[pageNum].fullSize.isValid());

    QSize boundedSize(_pages[pageNum].fullSize);
    boundedSize.scale(_displaySize, Qt::KeepAspectRatio);

    kDebug()<<"Comparing image size "<<_pages[pageNum].pixmap.size()<<"to display size "<<_displaySize<<endl;

    return _pages[pageNum].pixmap.size() == boundedSize;
}

void PageLoader::startZoomMode()
{
    Q_ASSERT(!_zoomMode);
    _zoomMode = true;
}
void PageLoader::stopZoomMode(int nextPage)
{
    Q_ASSERT(_zoomMode);
    _zoomMode = false;

    // Schedule the decoding of any unscaled pages
    int delta = 0;
    _bufferStart = nextPage;
    _bufferEnd = nextPage;
    for (int i = 0; i < BUFFER_SIZE && i < _pages.size(); i++) {
        if (_pages[nextPage + delta].pixmap.isNull() || !isPageScaled(nextPage + delta)) {
            startReadingPage(nextPage + delta);
        } else {
            // Make sure the buffer edges get set correctly
            if (nextPage + delta < _bufferStart) {
                _bufferStart = nextPage + delta;
            }
            if (nextPage + delta > _bufferEnd) {
                _bufferEnd = nextPage + delta;
            }
        }
        if (delta <= 0) {
            if (nextPage - delta + 1 >= _pages.size()) {
                delta--;
            } else {
                delta = -delta + 1;
            }
        } else {
            if (nextPage - delta < 0) {
                delta++;
            } else {
                delta = -delta;
            }
        }
    }
}
bool PageLoader::isZoomMode() const
{
    return _zoomMode;
}

const QPixmap &PageLoader::getPage(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());
    Q_ASSERT(!_pages[pageNum].pixmap.isNull());

    // Don't queue any decodes if zoom mode is active
    if (!_zoomMode) {
        // As we're getting this page, make sure we have a buffer of decoded pages to either side
        if (_bufferEnd - pageNum < BUFFER_SIZE/2) {
            for (int i = _bufferEnd + 1; i - pageNum <= BUFFER_SIZE/2 && i < _pages.size(); i++) {
                startReadingPage(i);
            }
        }

        if (pageNum - _bufferStart < BUFFER_SIZE/2) {
            for (int i = _bufferStart - 1; pageNum - i <= BUFFER_SIZE/2 && i >= 0; i--) {
                startReadingPage(i);
            }
        }
    }

    return _pages[pageNum].pixmap;
}

void PageLoader::startReadingPage(int pageNum)
{
    Q_ASSERT(pageNum < _pages.size());
    //Q_ASSERT(_pages[pageNum].pixmap.isNull());

    if (!_pages[pageNum].isLoading) {
        _pages[pageNum].isLoading = true;
        _pages[pageNum].loadingTime.restart();

        kDebug()<<"Queueing page "<<pageNum<<" decode"<<endl;

        // If we're in zoom mode, decode an unscaled version of the page
        if (_zoomMode) {
            // Note: if we're just resizing the window, maybe we don't need to scale from the image's
            //  full resolution. Then again, scaling from the full resolution will give the highest
            //  qualitity results
            if (_archiveMode) {
                _decodeWeaver.enqueue(new ExtractDecodeJob(pageNum, _pages[pageNum].path, QSize(), _archivePath));
            } else {
                _decodeWeaver.enqueue(new FileDecodeJob(pageNum, _pages[pageNum].path, QSize()));
            }
            // The buffer will have to be reaccessed after the zoom mode is finished

        } else {
            if (_archiveMode) {
                _decodeWeaver.enqueue(new ExtractDecodeJob(pageNum, _pages[pageNum].path, _displaySize, _archivePath));
            } else {
                _decodeWeaver.enqueue(new FileDecodeJob(pageNum, _pages[pageNum].path, _displaySize));
            }

            if (pageNum < _bufferStart) {
                _bufferStart = pageNum;
            }
            if (pageNum > _bufferEnd) {
                _bufferEnd = pageNum;
            }
        }
    }
}

void PageLoader::decodeDone(ThreadWeaver::Job *job)
{
    DecodeJob *decodeJob = qobject_cast<DecodeJob *>(job);

    Q_ASSERT(decodeJob != 0);

    int pageNum = decodeJob->pageNum();

    kDebug()<<"Decode job done"<<endl;
    kDebug()<<"Time elapsed (page "<<pageNum<<"): "<<_pages[pageNum].loadingTime.elapsed()<<" ms"<<endl;

    // Check that the decode suceeded
    if (decodeJob->image().isNull()) {
        emit pageReadFailed(pageNum);
        return;
    }

    QTime time;
    time.start();
    _pages[pageNum].pixmap = QPixmap::fromImage(decodeJob->image());
    kDebug()<<"QImage to QPixmap conversion: "<<time.elapsed()<<" ms"<<endl;

    _pages[pageNum].fullSize = decodeJob->fullImageSize();
    _pages[pageNum].isLoading = false;

    // Check that the image to pixmap conversion suceeded
    if (_pages[pageNum].pixmap.isNull()) {
        emit pageReadFailed(pageNum);
    } else {;
        emit pageRead(pageNum);
    }
}

void PageLoader::listingEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
    Q_ASSERT(job == _listJob);

    //kDebug()<<"Progress: "<<_listJob->percent()<<"%"<<endl<<"Entries: "<<list.size()<<endl;

    Page tempPage;
    KUrl tempPath;

    // Some directory residents have been found; go through the list looking for images
    for (KIO::UDSEntryList::const_iterator i = list.begin(); i != list.end(); i++) {
        if (!i->isDir()) {
            tempPath = _currentDir;
            tempPath.addPath(i->stringValue(KIO::UDS_NAME));
            tempPage.path = tempPath.toLocalFile();

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
        QString initialPath = _pages.front().path;
        initialPosition = 0;
        for (QList<Page>::iterator i = _pages.begin() + 1; i != _pages.end(); i++) {
            if (i->path == initialPath) {
                // The initial page has been found
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
        kDebug()<<"Path: "<<i->path<<endl;
    }
    emit pagesListed(initialPosition, _pages.size());

    // Start reading pages
    int delta = 0;
    _bufferStart = initialPosition;
    _bufferEnd = initialPosition;
    for (int i = 0; i < BUFFER_SIZE && i < _pages.size(); i++) {
        startReadingPage(initialPosition + delta);
        if (delta <= 0) {
            if (initialPosition - delta + 1 >= _pages.size()) {
                delta--;
            } else {
                delta = -delta + 1;
            }
        } else {
            if (initialPosition - delta < 0) {
                delta++;
            } else {
                delta = -delta;
            }
        }
    }

    // Start listing the parent directory
    KUrl parentDir = _currentDir;
    if (!parentDir.cd("..")) {
        kDebug()<<"Can't go up one directory from "<<_currentDir.prettyUrl()<<endl;
    } else {
        kDebug()<<"Parent dir: "<<parentDir<<endl;
    }
}

void PageLoader::extracterOutputText()
{
    if (_listingBodyFinished) {
        _extracterProcess.readAllStandardOutput();
        return;
    }

    //kDebug()<<"Output ready"<<endl;

    QByteArray output = _extracterProcess.readAllStandardOutput();
    int newLineIdx;

    //kDebug()<<"Got output: "<<QString(output)<<endl;

    while ((newLineIdx = output.indexOf('\n')) != -1) {
        // New line found
        // Fill a full line of input, excluding the new line
        _currentInputLine.append(output.left(newLineIdx));
        //kDebug()<<"Got full line of output: "<<QString(_currentInputLine)<<endl;

        if (!_listingBodyReached) {
            if (_currentInputLine.length() > 0 && _currentInputLine.count('-') == _currentInputLine.length()) {
                // We've reached the start of the listing
                _listingBodyReached = true;
            }
        } else {
            // A full line listing data has been recieved
            Q_ASSERT(_currentInputLine.length() > 0);
            if (_filenameLine) {
                if (_currentInputLine.count('-') == _currentInputLine.length()) {
                    // We've reached the end of the listing
                    // The rest of the data isn't useful
                    _listingBodyFinished = true;
                    return;
                }
                Q_ASSERT(_currentInputLine[0] == ' ');

                Page newPage;
                newPage.path = QString(_currentInputLine).trimmed();
                Q_ASSERT(newPage.path.length() != 0);
                _pages.append(newPage);
            } else {
                QString fullLine = _currentInputLine;
                QStringList data = fullLine.split(" ", QString::SkipEmptyParts);
                Q_ASSERT(data.size() == 9);

                // Check if the previous entry was a directory
                // Note: if rarred from windows, looks like ".D.....", from linux, looks like "drwxr-xr-x"
                QString permissions = data[5];
                if (permissions.contains('d', Qt::CaseInsensitive)) {
                    _pages.pop_back();
                }
            }
            _filenameLine = !_filenameLine;
        }

        // Start a new line of input, excluding the new line
        _currentInputLine = "";
        output = output.right(output.length() - (newLineIdx + 1));
    }

    // No new line, keep constructing a full line
    _currentInputLine.append(output);
}
void PageLoader::extracterErrorText()
{
    kDebug()<<"Error text ready"<<endl;
}
void PageLoader::extracterError(QProcess::ProcessError)
{
    kDebug()<<"Error"<<endl;
    Q_ASSERT(false);
}
void PageLoader::extracterFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_ASSERT(exitCode == 0);
    Q_ASSERT(exitStatus == QProcess::NormalExit);

    // Note: pages might not be in a good order, depending on the decompressor's
    //  "sorting" logic
    kDebug()<<"Total pages: "<<_pages.size()<<endl;
    for (QList<Page>::iterator i = _pages.begin(); i != _pages.end(); i++) {
        kDebug()<<"Path: "<<i->path<<endl;
    }
    emit pagesListed(0, _pages.size());

    // Start reading pages
    _bufferStart = 0;
    _bufferEnd = 0;
    for (int i = 0; i < BUFFER_SIZE && i < _pages.size(); i++) {
        startReadingPage(i);
    }
}


#include "pageloader.moc"
