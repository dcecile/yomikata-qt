#include "archivelister.h"

#include <QTextCodec>

#include "debug.h"

ArchiveLister::ArchiveLister(const QString &archivePath, QObject *parent)
    : QObject(parent)
{
    _archiveType = FileClassification::getArchiveType(archivePath);
    _archivePath = archivePath;

    // Connect to the extracter process
    connect(&_process, SIGNAL(readyReadStandardError()),
             this, SLOT(errorText()));
    connect(&_process, SIGNAL(error(QProcess::ProcessError)),
             this, SLOT(error(QProcess::ProcessError)));
    connect(&_process, SIGNAL(finished(int, QProcess::ExitStatus)),
             this, SLOT(finished(int, QProcess::ExitStatus)));
}

/**
 * @todo Test that this works.
 * @todo Disconnect during terminate.
 */
ArchiveLister::~ArchiveLister()
{
    // Check if the process is running
    if (_process.state() != QProcess::NotRunning)
    {
        // Disconnect

        // Send a SIGTERM signal
        debug()<<"Terminating archive lister process";
        _process.terminate();

        // Wait kindly for it to finish
        bool finished = _process.waitForFinished(0);

        // Kill the process if it's still running
        if (!finished)
        {
            debug()<<"Killing archive lister process";
            _process.kill();
        }
    }
}

void ArchiveLister::start()
{
    Q_ASSERT(_process.state() == QProcess::NotRunning);

    // Start a fresh listing
    _listingBodyReached = false;
    _filenameLine = true;
    _listingBodyFinished = false;
    _currentInputLine = "";

    // Determine the executable and parameters used to list the archive
    QString command;
    QStringList args;
    switch (_archiveType)
    {
        case FileClassification::Tar:
            command = "tar";
            args<<"-tvf";
            _numFields = 5;
            _sizeField = 2;
            break;
        case FileClassification::TarGz:
            command = "tar";
            args<<"-ztvf";
            _numFields = 5;
            _sizeField = 2;
            break;
        case FileClassification::TarBz:
            command = "tar";
            args<<"--bzip2"<<"-tvf";
            _numFields = 5;
            _sizeField = 2;
            break;
        case FileClassification::TarZ:
            command = "tar";
            args<<"-Ztvf";
            _numFields = 5;
            _sizeField = 2;
            break;
        case FileClassification::Zip:
            command = "unzip";
            args<<"-l"<<"-qq";
            _numFields = 3;
            _sizeField = 0;
            break;
        case FileClassification::Rar:
            command = "unrar";
            args<<"vr";
            break;
        default:
            Q_ASSERT(false);
    }
    args<<_archivePath;

    // Connect to the process
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(nonRarOutputText()));
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(rarOutputText()));

    if (_archiveType != FileClassification::Rar)
    {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                 this, SLOT(nonRarOutputText()));
    }
    else
    {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                 this, SLOT(rarOutputText()));
    }

    // Start the process listing
    _process.start(command, args);
}

void ArchiveLister::nonRarOutputText()
{
    QByteArray output = _process.readAllStandardOutput();
    int newLineIdx;

    int size;
    bool parsed;

    //debug()<<"Got output:"<<QString(output);

    while ((newLineIdx = output.indexOf('\n')) != -1)
    {
        // New line found
        // Fill a full line of input, excluding the new line
        _currentInputLine.append(output.left(newLineIdx));

        QString fullLine = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine);
        QStringList data = fullLine.split(" ");

        // Skip fields before size field
        for (int i = 0; i < _sizeField; i++)
        {
            while (data.front().isEmpty())
            {
                data.pop_front();
            }

            data.pop_front();
        }

        // Skip space before size field
        while (data.front().isEmpty())
        {
            data.pop_front();
        }

        // A size 0 is probably a directory, maybe an empty file; ignore this entry
        if (data.front() != "0")
        {
            // Store the file size
            size = data.front().toInt(&parsed);
            Q_ASSERT(parsed);

            // Skip the rest of the fields before name field
            data.pop_front();
            for (int i = 0; i < _numFields - _sizeField - 1; i++)
            {
                while (data.front().isEmpty())
                {
                    data.pop_front();
                }

                data.pop_front();
            }

            // Put the name back together and trim whitespace
            QString filename = data.join(" ").trimmed();

            if (FileClassification::isImageFile(filename))
            {
                // This is an image file

                // Unzip has huge problems with filenames, so try to clean them up a bit
                // For example, it can't handle '[' or ']' in the path
                //  or filenames encoded from a non-UTF charset like Shift-JIS
                if (_archiveType == FileClassification::Zip)
                {
                    filename = cleanZipFilename(filename);
                }

                emit entryFound(filename, size, 0);
            }
        }

        // Start a new line of input, excluding the new line
        _currentInputLine = "";
        output = output.right(output.length() - (newLineIdx + 1));
    }

    // No new line, keep constructing a full line
    _currentInputLine.append(output);
}

void ArchiveLister::rarOutputText()
{
    if (_listingBodyFinished)
    {
        _process.readAllStandardOutput();
        return;
    }

    //debug()<<"Output ready";

    QByteArray output = _process.readAllStandardOutput();
    int newLineIdx;

    bool parsed;

    //debug()<<"Got output:"<<QString(output);

    while ((newLineIdx = output.indexOf('\n')) != -1)
    {
        // New line found
        // Fill a full line of input, excluding the new line
        _currentInputLine.append(output.left(newLineIdx));
        //debug()<<"Got full line of output: "<<QString(_currentInputLine);

        if (!_listingBodyReached)
        {
            if (_currentInputLine.length() > 0 && _currentInputLine.count('-') == _currentInputLine.length())
            {
                // We've reached the start of the listing
                _listingBodyReached = true;
            }
        }
        else
        {
            // A full line listing data has been recieved
            // When looking at the actual data, make sure to do a conversion to UTF-8
            Q_ASSERT(_currentInputLine.length() > 0);

            if (_filenameLine)
            {
                if (_currentInputLine.count('-') == _currentInputLine.length()) {
                    // We've reached the end of the listing
                    // The rest of the data isn't useful
                    _listingBodyFinished = true;
                    return;
                }

                Q_ASSERT(_currentInputLine[0] == ' ');

                _rarFileName = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine).trimmed();
                Q_ASSERT(_rarFileName.length() != 0);
            }
            else
            {
                Q_ASSERT(_currentInputLine[0] == ' ');

                QString fullLine = QTextCodec::codecForName("utf-8")->toUnicode(_currentInputLine);
                QStringList data = fullLine.split(" ", QString::SkipEmptyParts);
                Q_ASSERT(data.size() == 9);

                // Check if the previous entry was a directory
                // Note: all directories will have size 0
                // And check that the file is an image
                QString size = data[1];
                if (size != "0" && FileClassification::isImageFile(_rarFileName))
                {
                    int parsedSize = size.toInt(&parsed);
                    Q_ASSERT(parsed);

                    emit entryFound(_rarFileName, parsedSize, 0);
                }
            }

            // Alternate to a non-filename line
            _filenameLine = !_filenameLine;
        }

        // Start a new line of input, excluding the new line
        _currentInputLine = "";
        output = output.right(output.length() - (newLineIdx + 1));
    }

    // No new line, keep constructing a full line
    _currentInputLine.append(output);
}

void ArchiveLister::errorText()
{
    debug()<<"extracter error:"<<QTextCodec::codecForName("utf-8")->toUnicode(_process.readAllStandardError());
}

void ArchiveLister::error(QProcess::ProcessError error)
{
    debug()<<"Error"<<error;
    Q_ASSERT(false);
}

void ArchiveLister::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_ASSERT(exitCode == 0);
    Q_ASSERT(exitStatus == QProcess::NormalExit);

    // Note: pages might not be in a good order, depending on the decompressor's
    //  "sorting" logic
    emit finished();
}

QString ArchiveLister::cleanZipFilename(const QString &filename)
{
    QString fixed = filename;

    // If the filename has any "bad" characters, replace them with the wildcard '*'
    for (int j = 0; j < fixed.length(); j++)
    {
        //debug()<<"Is letter / number "<<fixed[j].isLetterOrNumber();
        if (!fixed[j].isLetterOrNumber())
        {
            const char ascii = fixed[j].toAscii();
            //debug()<<"Char"<<j<<fixed[j]<<"-"<<int((unsigned char)ascii);
            if (ascii == 0 || !(ascii == ' ' || ascii == '_' || ascii == '/' || ascii == '.'))
            {
                fixed[j] = '*';
            }
        }
    }

    return fixed;
}

#include "archivelister.moc"
