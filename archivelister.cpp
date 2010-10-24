#include "archivelister.h"

#include <QRegExp>
#include <QTextCodec>

#include "archive.h"
#include "debug.h"

ArchiveLister::ArchiveLister(const Archive &archive, QObject *parent)
    : QObject(parent), _archive(archive)
{
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
        bool finished = _process.waitForFinished(KILL_WAIT);

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
    QStringList args;
    switch (_archive.type())
    {
        case Archive::SevenZip:
            args<<"l"<<"-slt";
            break;
        case Archive::Tar:
            args<<"-tvf";
            _numFields = 5;
            _sizeField = 2;
            break;
        case Archive::Zip:
            args<<"-l"<<"-qq";
            _numFields = 3;
            _sizeField = 0;
            break;
        case Archive::Rar:
            args<<"vr";
            break;
        default:
            Q_ASSERT(false);
    }
    args<<_archive.filename();

    // Connect to the process
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(defaultParser()));
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(rarParserText()));
    disconnect(&_process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(sevenZipParser()));

    // Set the parser
    if (_archive.type() == Archive::SevenZip)
    {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                 this, SLOT(sevenZipParser()));
    }
    else if (_archive.type() == Archive::Rar)
    {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                 this, SLOT(rarParserText()));
    }
    else
    {
        connect(&_process, SIGNAL(readyReadStandardOutput()),
                 this, SLOT(defaultParser()));
    }

    // Start the process listing
    _process.start(_archive.programPath(), args);
}

void ArchiveLister::sevenZipParser()
{
    _currentInputLine.append(_process.readAllStandardOutput());
    int newLineIdx;

    while ((newLineIdx = _currentInputLine.indexOf("\n\n")) != -1)
    {
        sevenZipParserBlock(
            _currentInputLine.left(newLineIdx));
        _currentInputLine.remove(0, newLineIdx + 2);
    }
}

void ArchiveLister::sevenZipParserBlock(const QByteArray &block)
{
    // Each line should have a "Key = Value" entry (store them in a map)
    // (Note: the 7z prelude will simply not match this pattern)
    QList<QByteArray> lines = block.split('\n');
    QMap<QByteArray, QByteArray> attributes;

    foreach (QByteArray line, lines)
    {
        int equals = line.indexOf(" = ");
        if (equals != -1)
        {
            attributes[line.left(equals)] = line.mid(equals + 3);
        }
    }

    // If this block is for a file (it has all of the attributes), go ahead
    // and classify it as an archive entry
    if (attributes.contains("Path")
        && attributes.contains("Size")
        && attributes.contains("Packed Size"))
    {
        const QByteArray &filename = attributes["Path"];

        if (FileClassification::isImageFile(filename))
        {
            emit entryFound(
                filename,
                attributes["Packed Size"].toInt(),
                attributes["Size"].toInt());
        }
    }
}

void ArchiveLister::defaultParser()
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

        // Trim the full line
        _currentInputLine = _currentInputLine.trimmed();

        // Skip blank lines
        if (_currentInputLine.length() == 0)
        {
            // Skip this input
            output = output.right(output.length() - (newLineIdx + 1));

            // Don't process this line
            continue;
        }

        QString fullLine = QTextCodec::codecForLocale()->toUnicode(_currentInputLine);
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

            if (FileClassification::isImageFile(filename.toLocal8Bit()))
            {
                // This is an image file

                // Unzip has huge problems with filenames, so try to clean them up a bit
                // For example, it can't handle '[' or ']' in the path
                //  or filenames encoded from a non-UTF charset like Shift-JIS
                if (_archive.type() == Archive::Zip)
                {
                    filename = cleanZipFilename(filename);
                }

                emit entryFound(filename.toLocal8Bit(), size, 0);
            }
        }

        // Start a new line of input, excluding the new line
        _currentInputLine = "";
        output = output.right(output.length() - (newLineIdx + 1));
    }

    // No new line, keep constructing a full line
    _currentInputLine.append(output);
}

void ArchiveLister::rarParserText()
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

        // Trim the full line
        _currentInputLine = _currentInputLine.trimmed();

        //debug()<<"Length"<<_currentInputLine.length();

        // Skip blank lines
        if (_currentInputLine.length() == 0)
        {
            // Skip this input
            output = output.right(output.length() - (newLineIdx + 1));

            // Don't process this line
            continue;
        }

        if (!_listingBodyReached)
        {
            if (_currentInputLine.count('-') == _currentInputLine.length())
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

                _rarFileName = QTextCodec::codecForLocale()->toUnicode(_currentInputLine);
                Q_ASSERT(_rarFileName.length() != 0);
            }
            else
            {
                QString fullLine = QTextCodec::codecForLocale()->toUnicode(_currentInputLine);
                QStringList data = fullLine.split(" ", QString::SkipEmptyParts);
                Q_ASSERT(data.size() == 9);

                // Check if the previous entry was a directory
                // Note: all directories will have size 0
                // And check that the file is an image
                QString size = data[1];

                if (size != "0" && FileClassification::isImageFile(_rarFileName.toLocal8Bit()))
                {
                    int parsedSize = size.toInt(&parsed);
                    Q_ASSERT(parsed);

                    emit entryFound(_rarFileName.toLocal8Bit(), parsedSize, 0);
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
