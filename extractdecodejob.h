#ifndef EXTRACTDECODEJOB_H
#define EXTRACTDECODEJOB_H

#include <QImage>
#include <KUrl>
#include <threadweaver/Job.h>

#include "decodejob.h"

class ExtractDecodeJob : public DecodeJob
{
    Q_OBJECT

public:
    ExtractDecodeJob(int pageNum, const QString &path, const QSize &boundingSize, const QString &archive);

private:
    void run();

private:
    QString _archive;
};

#endif
