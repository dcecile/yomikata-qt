#include "archive.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTime>

#include "debug.h"

Archive::Archive(QObject *parent)
    : QObject(parent)
{
    // No type set
    _type = InvalidArchiveType;

    // Initialize by the program settings
    _settings.beginGroup("archivers");

    //debug()<<"Settings file"<<_settings.fileName();

    // Grab the program paths
    _programPaths[SevenZip] = _settings.value("sevenZipPath", "7z").toString();
    _programPaths[Tar] = _settings.value("tarPath", "tar").toString();
    _programPaths[Zip] = _settings.value("zipPath", "unzip").toString();
    _programPaths[Rar] = _settings.value("rarPath", "unrar").toString();

    // Check if existence has been checked for yet
    bool tested = _settings.value("tested", false).toBool();

    // If they were tested
    if (tested)
    {
        // Grab the results from the settings
        _programExists[SevenZip] = _settings.value("sevenZipExists", false).toBool();
        _programExists[Tar] = _settings.value("tarExists", false).toBool();
        _programExists[Zip] = _settings.value("zipExists", false).toBool();
        _programExists[Rar] = _settings.value("rarExists", false).toBool();
        _sevenZipRarExists = _settings.value("sevenZipRarExists", false).toBool();
    }
    // If not
    else
    {
        // Do a full test
        QTime clock;
        clock.start();
        testPrograms();
        debug()<<"Tested"<<clock.elapsed();
    }
}

/**
 * @todo Do a Mac (or other?) check for p7zip-rar
 */
void Archive::testPrograms()
{
    // Start a process for each program
    QProcess programs[InvalidArchiveType];

    programs[SevenZip].start(_programPaths[SevenZip], QStringList());
    programs[Tar].start(_programPaths[Tar], QStringList()<<"--version");
    programs[Zip].start(_programPaths[Zip], QStringList()<<"-v");
    programs[Rar].start(_programPaths[Rar], QStringList());

    // Wait for each process to finish
    for (int i = 0; i < InvalidArchiveType; i++)
    {
        const int MAX_WAIT = 200;
        bool waited;

        // Start without errors
        waited = programs[i].waitForStarted(MAX_WAIT);

        // Finish without errors
        waited = waited && programs[i].waitForFinished(MAX_WAIT);

        // If finished without errors, the program is OK to use
        _programExists[i] = waited;
    }

    // Save the results to the settings
    _settings.setValue("sevenZipExists", _programExists[SevenZip]);
    _settings.setValue("sevenZipPath", _programPaths[SevenZip]);

    _settings.setValue("tarExists", _programExists[Tar]);
    _settings.setValue("tarPath", _programPaths[Tar]);

    _settings.setValue("zipExists", _programExists[Zip]);
    _settings.setValue("zipPath", _programPaths[Zip]);

    _settings.setValue("rarExists", _programExists[Rar]);
    _settings.setValue("rarPath", _programPaths[Rar]);

    // Check for the p7zip-rar package
#ifndef Q_OS_WIN32
    if (!_programExists[SevenZip])
    {
        // Can't exist without 7z
        _sevenZipRarExists = false;
    }
    else
    {
        // Find the 7z path
        QFileInfo fullSevenZipPath(_programPaths[SevenZip]);

        // If no full path specified, look through the environment paths
        if (!fullSevenZipPath.isAbsolute())
        {
            QStringList environment = QProcess::systemEnvironment();
            QStringList systemPath;

            for (int i = 0; i < environment.size(); i++)
            {
                if (environment[i].startsWith("PATH="))
                {
                    systemPath = environment[i].right(environment[i].length() - 5).split(':');
                    break;
                }
            }

            for (int i = 0; i < systemPath.size(); i++)
            {
                QFileInfo testPath(systemPath[i], _programPaths[SevenZip]);

                if (testPath.exists())
                {
                    fullSevenZipPath = testPath;
                    break;
                }
            }
        }

        // Navigate to the correct folder
        QDir codecsFolder(fullSevenZipPath.dir());
        codecsFolder.cd("../lib/p7zip/Codecs");

        // Test for the shared library
        _sevenZipRarExists = QFileInfo(codecsFolder, "Rar29.so").exists();
    }
#else
    // On Windows, assume it exists
    _sevenZipRarExists = _programExists[SevenZip];
#endif

    // Save the 7z-rar setting
    _settings.setValue("sevenZipRarExists", _sevenZipRarExists);

    // Programs have been tested
    _settings.setValue("tested", true);

    // Make sure the settings get written
    _settings.sync();
}

Archive::~Archive()
{
}

void Archive::reset(const QString &filename)
{
    const struct
    {
        const char *ext;
        Type type;
    }
    ARCHIVE_TYPES[] =
    {
        {".tar", Tar},
        {".zip", Zip},
        {".cbz", Zip},
        {".rar", Rar},
        {".cbr", Rar},
        {".7z", SevenZip},
        // Ace
        // Lzh
        {"", InvalidArchiveType},
    };

    _filename = filename;
    _type = InvalidArchiveType;

    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        if (_filename.endsWith(ARCHIVE_TYPES[i].ext, Qt::CaseInsensitive))
        {
            _type = ARCHIVE_TYPES[i].type;
        }
    }

    switch (_type)
    {
    case SevenZip:
        if (!_programExists[SevenZip])
        {
            _type = InvalidArchiveType;
        }
        break;
    case Tar:
        if (!_programExists[Tar])
        {
            if (!_programExists[SevenZip])
            {
                _type = InvalidArchiveType;
            }
            else
            {
                _type = SevenZip;
            }
        }
        break;
    case Zip:
        if (!_programExists[Zip])
        {
            if (!_programExists[SevenZip])
            {
                _type = InvalidArchiveType;
            }
            else
            {
                _type = SevenZip;
            }
        }
        break;
    case Rar:
        if (!_programExists[Rar])
        {
            if (!_programExists[SevenZip] || !_sevenZipRarExists)
            {
                _type = InvalidArchiveType;
            }
            else
            {
                _type = SevenZip;
            }
        }
        break;
    case InvalidArchiveType:
        break;
    }

    debug()<<"Archive type"<<_type;
    Q_ASSERT(_type != InvalidArchiveType);
}

const QString &Archive::filename() const
{
    return _filename;
}

Archive::Type Archive::type() const
{
    return _type;
}

const QString &Archive::programPath() const
{
    return _programPaths[_type];
}

#include "archive.moc"
