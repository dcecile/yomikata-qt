#ifndef EXTRACTLISTER_H
#define EXTRACTLISTER_H

#include <QProcess>
#include <QStringList>
#include <QByteArray>

#include "fileinfo.h"

class ExtractLister : public QObject
{
    Q_OBJECT

public:
    ExtractLister();

    void list(FileInfo::ArchiveType archiveType, const QString &archivePath);

signals:
    void listBuilt(int initialPosition, const QStringList &files);

private slots:
    void rarOutputText();
    void nonRarOutputText();
    void errorText();
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QStringList _fileList;

    QProcess _process;

    int _numFields;
    int _sizeField;

    bool _listingBodyReached;
    bool _listingBodyFinished;
    bool _filenameLine;
    QByteArray _currentInputLine;
};

#endif
