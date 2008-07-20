#ifndef FILEDECODEJOB_H
#define FILEDECODEJOB_H

#include <QImage>
#include <KUrl>
#include <threadweaver/Job.h>

#include "decodejob.h"

class FileDecodeJob : public DecodeJob
{
    Q_OBJECT

public:
    FileDecodeJob(int pageNum, const QString &path, const QSize &boundingSize, bool highPriority);

private:
    void run();
};

#endif
