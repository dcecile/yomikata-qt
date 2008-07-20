#ifndef EXTRACTDECODETHREAD_H
#define EXTRACTDECODETHREAD_H

#include "decodethread.h"
#include "fileinfo.h"

class ExtractDecodeThread : public DecodeThread
{
    Q_OBJECT

public:
    ExtractDecodeThread(PageCache *pageCache);
    ~ExtractDecodeThread();

    void setArchive(FileInfo::ArchiveType archiveType, const QString &archive);

private:
    void decode(int pageNum, const QString &path, QSize fullSize, const QSize &boundingSize);

private:
    FileInfo::ArchiveType _archiveType;
    QString _archive;
};

#endif
