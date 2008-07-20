#ifndef FILELISTER_H
#define FILELISTER_H

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <QStringList>
#include <QTime>

class FileLister : public QObject
{
    Q_OBJECT

public:
    FileLister();

    void list(const QString &initialFile);

signals:
    void listBuilt(QStringList files, QString startPageName);

private slots:
    void entries(KIO::Job *job, const KIO::UDSEntryList &list);
    void finished(KJob *job);

public:
    QStringList _fileList;
    QString _initialFile;

    KIO::ListJob *_listJob;
    KUrl _currentDir;

    QTime _listingTime;
};

#endif
