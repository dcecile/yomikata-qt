#ifndef EXTRACTLISTER_H
#define EXTRACTLISTER_H

#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QTime>

#include "fileinfo.h"

class ExtractLister : public QObject
{
    Q_OBJECT

public:
    ExtractLister();

    void list(FileInfo::ArchiveType archiveType, const QString &archivePath);

signals:
    void listBuilt(QStringList files, QString startPageName);

private slots:
    void rarOutputText();
    void nonRarOutputText();
    void errorText();
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void cleanZipFilenames();

private:
    QStringList _fileList;

    QProcess _process;

    FileInfo::ArchiveType _archiveType;

    int _numFields;
    int _sizeField;

    bool _listingBodyReached;
    bool _listingBodyFinished;
    bool _filenameLine;
    QByteArray _currentInputLine;

    QTime _listingTime;
};

#endif
