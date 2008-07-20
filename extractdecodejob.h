#ifndef EXTRACTDECODEJOB_H
#define EXTRACTDECODEJOB_H

#include <QImage>
#include <KUrl>
#include <threadweaver/Job.h>

#include "decodejob.h"
#include "fileinfo.h"

class ExtractDecodeJob : public DecodeJob
{
    Q_OBJECT

public:
    ExtractDecodeJob(int pageNum, const QString &path, const QSize &boundingSize, FileInfo::ArchiveType archiveType, const QString &archive);

private:
    void run();

private:
    FileInfo::ArchiveType _archiveType;
    QString _archive;
};

#endif
