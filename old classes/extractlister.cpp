#include <KDebug>
#include <QTextCodec>
#include <QTextStream>

#include "extractlister.h"

ExtractLister::ExtractLister()
{
    // Connect to the extracter process
    connect(&_process, SIGNAL(readyReadStandardError()),
            this, SLOT(errorText()));
    connect(&_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(error(QProcess::ProcessError)));
    connect(&_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(finished(int, QProcess::ExitStatus)));
}

void ExtractLister::list(FileInfo::ArchiveType archiveType, const QString &archivePath)
{
    Q_ASSERT(_process.state() == QProcess::NotRunning);

    // Check that the temp directory is writable
    //QDir::tempPath();

    // Start a fresh listing
    _fileList.clear();
    _listingBodyReached = false;
    _filenameLine = true;
    _listingBodyFinished = false;
    _currentInputLine = "";
    _listingTime.restart();

    // List the contents of the archive

    QString command;
    QStringList args;

    _archiveType = archiveType;
    switch (_archiveType) {
    case FileInfo::Tar:
        command = "tar";
        args<<"-tvf";
        _numFields = 5;
        _sizeField = 2;
        break;
    case FileInfo::TarGz:
        command = "tar";
        args<<"-ztvf";
        _numFields = 5;
        _sizeField = 2;
        break;
    case FileInfo::TarBz:
        command = "tar";
        args<<"--bzip2"<<"-tvf";
        _numFields = 5;
        _sizeField = 2;
        break;
    case FileInfo::TarZ:
        command = "tar";
        args<<"-Ztvf";
        _numFields = 5;
        _sizeField = 2;
        break;
    case FileInfo::Zip:
        command = "unzip";
        args<<"-l"<<"-qq";
        _numFields = 3;
        _sizeField = 0;
        break;
    case FileInfo::Rar:
        command = "unrar";
        args<<"vr";
        break;
    default:
        Q_ASSERT(false);
    }
    args<<archivePath;

    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(nonRarOutputText()));
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(rarOutputText()));
    if (_archiveType != FileInfo::Rar) {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(nonRarOutputText()));
    } else {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(rarOutputText()));
    }
    _process.start(command, args);

    kDebug()<<"Started archive listing for "<<archivePath<<endl;
}

void ExtractLister::nonRarOutputText()
{
    QByteArray output = _process.readAllStandardOutput();
    int newLineIdx;

    //kDebug()<<"Got output: "<<QString(output)<<endl;

    while ((newLineIdx = output.indexOf('\n')) != -1) {
        // New line found
        // Fill a full line of input, excluding the new line
        _currentInputLine.append(output.left(newLineIdx));

        /*
        QTextStream stream(_currentInputLine);
        stream.setCodec("utf-8");
        QString temp;

        do {
            stream>>temp;
            kDebug()<<"Pulled string "<<temp<<endl;
            kDebug()<<"Stream status "<<stream.status()<<endl;
        } while (stream.status() != QTextStream::ReadPastEnd);
        Q_ASSERT(false);
        //*/

        //*
        //kDebug()<<"Got full line of output: "<<QString(_currentInputLine)<<endl;
        //kDebug()<<"Line length "<<_currentInputLine.length()<<endl;
        //QString fullLine = _currentInputLine;
        QString fullLine = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine);
        //QString fullLine = QTextCodec::codecForName("Shift-JIS")->toUnicode(_currentInputLine);
        QStringList data = fullLine.split(" ");

        // Skip fields before size field
        for (int i = 0; i < _sizeField; i++) {
            while (data.front().isEmpty()) {
                data.pop_front();
            }
            data.pop_front();
        }

        // Skip space before size field
        while (data.front().isEmpty()) {
            data.pop_front();
        }

        // A size 0 is probably a directory, maybe an empty file; ignore this entry
        if (data.front() != "0") {

            // Skip the rest of the fields before name field
            data.pop_front();
            for (int i = 0; i < _numFields - _sizeField - 1; i++) {
                while (data.front().isEmpty()) {
                    data.pop_front();
                }
                data.pop_front();
            }

            // Put the name back together and trim whitespace
            QString filename = data.join(" ").trimmed();

            if (FileInfo::isImageFile(filename)) {
                // This is an image file
                //kDebug()<<"Filename length "<<filename.length()<<endl;
                _fileList<<filename;
            }
        }
        //*/

        // Start a new line of input, excluding the new line
        _currentInputLine = "";
        output = output.right(output.length() - (newLineIdx + 1));
    }

    // No new line, keep constructing a full line
    _currentInputLine.append(output);
}
void ExtractLister::rarOutputText()
{
    if (_listingBodyFinished) {
        _process.readAllStandardOutput();
        return;
    }

    //kDebug()<<"Output ready"<<endl;

    QByteArray output = _process.readAllStandardOutput();
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
            // When looking at the actual data, make sure to do a conversion to UTF16
            Q_ASSERT(_currentInputLine.length() > 0);
            if (_filenameLine) {
                if (_currentInputLine.count('-') == _currentInputLine.length()) {
                    // We've reached the end of the listing
                    // The rest of the data isn't useful
                    _listingBodyFinished = true;
                    return;
                }
                Q_ASSERT(_currentInputLine[0] == ' ');

                QString filename = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine).trimmed();
                Q_ASSERT(filename.length() != 0);

                _fileList<<filename;

            } else {
                Q_ASSERT(_currentInputLine[0] == ' ');

                QString fullLine = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine);
                QStringList data = fullLine.split(" ", QString::SkipEmptyParts);
                Q_ASSERT(data.size() == 9);

                // Check if the previous entry was a directory
                // Note: all directories will have size 0
                QString size = data[1];
                if (size == "0") {
                    _fileList.pop_back();

                // Check that the file is an image
                } else if (!FileInfo::isImageFile(_fileList.back())) {
                    _fileList.pop_back();
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

void ExtractLister::errorText()
{
    kDebug()<<"extracter: "<<QTextCodec::codecForName("utf-8")->toUnicode(_process.readAllStandardError())<<endl;
}

void ExtractLister::error(QProcess::ProcessError error)
{
    kDebug()<<"Error"<<error<<endl;
    Q_ASSERT(false);
}

void ExtractLister::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_ASSERT(exitCode == 0);
    Q_ASSERT(exitStatus == QProcess::NormalExit);

    if (_archiveType == FileInfo::Zip) {
        // Unzip has huge problems with filenames, so try to clean them up a bit
        // For example, it can't handle '[' or ']' in the path
        //  or filenames encoded from a non-UTF charset like Shift-JIS
        cleanZipFilenames();
    }

    kDebug()<<"Listing finished: "<<_listingTime.elapsed()<<" ms"<<endl;

    // Note: pages might not be in a good order, depending on the decompressor's
    //  "sorting" logic
    emit listBuilt(_fileList, 0);
}

void ExtractLister::cleanZipFilenames()
{
    QString file;
    bool changed;
    for (QStringList::iterator i = _fileList.begin(); i != _fileList.end(); i++) {
        file = *i;
        changed = false;
        //kDebug()<<"Looking at file "<<file<<endl;

        // If the filename has any "bad" characters, replace them with the wildcard '*'
        for (int j=0; j<file.length(); j++) {
            //kDebug()<<"Is letter / number "<<file[j].isLetterOrNumber()<<endl;
            if (!file[j].isLetterOrNumber()) {
                const char ascii = file[j].toAscii();
                //kDebug()<<"Char "<<j<<" "<<file[j]<<"- "<<int((unsigned char)ascii)<<endl;
                if (ascii == 0 || !(ascii == ' ' || ascii == '_' || ascii == '/' || ascii == '.')) {
                    file[j] = '*';
                    changed = true;
                }
            }
        }

        // Collapse any wildcard characters
        if (changed) {
            *i = file;//file.split("*", QString::SkipEmptyParts).join("*");
        }
        //kDebug()<<"New name "<<*i<<endl;
        //Q_ASSERT(false);
    }
}

#include "extractlister.moc"
