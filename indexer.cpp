#include "indexer.h"

#include <algorithm>

#include "archivelister.h"
#include "debug.h"

using std::sort;

Indexer::Indexer(const Archive &archive, QObject *parent)
    : QObject(parent), _archive(archive)
{
    _archiveLister = NULL;
}

Indexer::~Indexer()
{
}

void Indexer::reset()
{
    // Stop any current lister
    if (_archiveLister != NULL)
    {
        delete _archiveLister;
    }

    // Clear the current indexer
    _files.clear();

    // Create a new archive lister
    _archiveLister = new ArchiveLister(_archive, this);

    // Connect to it
    connect(_archiveLister, SIGNAL(entryFound(const QString &, int, int)),
            SLOT(entryFound(const QString &, int, int)));
    connect(_archiveLister, SIGNAL(finished()), SLOT(listingFinished()));

    // Start it
    _archiveLister->start();

    // Start timing
    _listingTime.restart();
}

int Indexer::numPages() const
{
    return _files.size();
}

QString Indexer::pageName(int indexer) const
{
    Q_ASSERT(indexer >= 0 && indexer < (int) _files.size());
    return _files[indexer].name;
}

void Indexer::entryFound(const QString &filename, int compressedSize, int uncompressedSize)
{
    // Add the entry to the list
    FileInfo temp;
    temp.name = filename;
    temp.compressedSize = compressedSize;
    temp.uncompressedSize = uncompressedSize;
    _files.push_back(temp);
}

bool Indexer::FileInfo::operator < (const Indexer::FileInfo &other) const
{
    return QString::localeAwareCompare(name, other.name) < 0;
}

void Indexer::listingFinished()
{
    // Deallocate the lister
    _archiveLister->deleteLater();
    _archiveLister = NULL;

    // Sort all the entries
    sort(_files.begin(), _files.end());

    // Notify the steward
    debug()<<"Listing finished:"<<_listingTime.elapsed()<<" ms"
            <<"--"<<_files.size()<<"entries";
    emit built();
}

#include "indexer.moc"
