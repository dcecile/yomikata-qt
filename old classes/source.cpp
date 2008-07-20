#include "source.h"

#include "archivelister.h"
#include "extractdecodethread.h"

Source::Source(QObject *parent)
    : QObject(parent)
{
}

void Source::set(const QString &initialFile)
{
    // Determine file characteristics
    _fileName = initialFile;
    _isArchive = FileClassifier::isArchiveFile(_fileName);
    if (_isArchive) {
        _archiveType = FileClassifier::getArchiveType(_fileName);
    }
}

Lister *Source::createLister()
{
    return new ArchiveLister(_archiveType, _fileName, this);
}

DecodeThread *Source::createDecodeThread()
{
    return new ExtractDecodeThread(_archiveType, _fileName, this);
}

Source::~Source()
{
}

#include "source.moc"
