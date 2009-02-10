#ifndef INDEXER_H
#define INDEXER_H

#include <QObject>

#include <QTime>

#include <vector>

using std::vector;

class Archive;
class ArchiveLister;

/**
 * @todo Use QByteArray for the filenames (to overcome text codec errors)
 */
class Indexer : public QObject
{
    Q_OBJECT

public:
    Indexer(const Archive &archive, QObject *parent = NULL);
    ~Indexer();

    void reset();

    int numPages() const;
    QString pageName(int indexer) const;

signals:
    void built();

private slots:
    void entryFound(const QString &filename, int compressedSize, int uncompressedSize);
    void listingFinished();

private:
    struct FileInfo
    {
        QString name;
        int compressedSize;
        int uncompressedSize;

        bool operator < (const FileInfo &other) const;
    };

private:
    const Archive &_archive;
    vector<FileInfo> _files;

    ArchiveLister *_archiveLister;
    QTime _listingTime;
};

#endif
