#include "indexer.h"

#include "archivelister.h"

Indexer::Indexer(QObject *parent)
    : QObject(parent)
{
    _archiveLister = NULL;
}

Indexer::~Indexer()
{
}

void Indexer::reset(const QString &filename)
{
    // Stop any current lister
    if (_archiveLister != NULL)
    {
        delete _archiveLister;
    }

    // Clear the current indexer
    _files.clear();

    // Create a new archive lister
    _archiveLister = new ArchiveLister(filename, this);

    // Connect to it
    connect(_archiveLister, SIGNAL(entryFound(const QString &, int, int)),
            SLOT(entryFound(const QString &, int, int)));
    connect(_archiveLister, SIGNAL(finished()), SLOT(listingFinished()));

    // Start it
    _archiveLister->start();
}

int Indexer::numPages() const
{
    return _files.size();
}

QString Indexer::pageName(int indexer) const
{
    Q_ASSERT(indexer >= 0 && indexer < _files.size());
    return _files[indexer].name;
}

void Indexer::entryFound(const QString &filename, int compressedSize, int uncompressedSize)
{
}

void Indexer::listingFinished()
{
}

#include "indexer.moc"
