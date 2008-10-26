#ifndef INDEXER_H
#define INDEXER_H

#include <QObject>

#include <vector>

using std::vector;

class ArchiveLister;

class Indexer : public QObject
{
    Q_OBJECT

public:
    Indexer(QObject *parent = NULL);
    ~Indexer();

    void reset(const QString &filename);

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

        bool operator < (const FileInfo &other);
    };

private:
    vector<FileInfo> _files;

    ArchiveLister *_archiveLister;
};

#endif
