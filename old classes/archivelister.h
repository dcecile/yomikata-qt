#ifndef ARCHIVELISTER_H
#define ARCHIVELISTER_H

#include "lister.h"

#include <QProcess>
#include <QTime>

#include "fileclassifier.h"

/**
 * @ingroup mod_filelist
 * @brief Gets the list of files in an archive
 */
class ArchiveLister : public Lister
{
    Q_OBJECT

public:
    ArchiveLister(FileClassifier::ArchiveType archiveType, const QString &archivePath, QObject *parent);

    void beginListing();

    ~ArchiveLister();

private slots:
    void rarOutputText();
    void nonRarOutputText();
    void errorText();
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void cleanZipFilenames();

private:
    FileClassifier::ArchiveType _archiveType;
    QString _archivePath;

    QStringList _fileList;
    vector<int> _fileSizes;

    QProcess _process;

    int _numFields;
    int _sizeField;

    bool _listingBodyReached;
    bool _listingBodyFinished;
    bool _filenameLine;
    QByteArray _currentInputLine;

    QTime _listingTime;
};

#endif
