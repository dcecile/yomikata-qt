#ifndef ARCHIVELISTER_H
#define ARCHIVELISTER_H

#include <QObject>

#include <QProcess>

#include <vector>

#include "fileclassification.h"

using std::vector;

/**
 * @todo Cancel if being deconstructed.
 * @todo Allow parsing/program errors.
 */
class ArchiveLister : public QObject
{
    Q_OBJECT

public:
    ArchiveLister(const QString &archivePath, QObject *parent);
    ~ArchiveLister();

    void start();

signals:
    void entryFound(const QString &filename, int compressedSize, int uncompressedSize);
    void finished();

private slots:
    void rarOutputText();
    void nonRarOutputText();
    void errorText();
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString cleanZipFilename(const QString &filename);

private:
    FileClassification::ArchiveType _archiveType;
    QString _archivePath;

    QProcess _process;

    int _numFields;
    int _sizeField;

    bool _listingBodyReached;
    bool _listingBodyFinished;
    bool _filenameLine;
    QByteArray _currentInputLine;

    QString _rarFileName;
};

#endif
