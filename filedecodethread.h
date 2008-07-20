#ifndef FILEDECODETHREAD_H
#define FILEDECODETHREAD_H

#include "decodethread.h"

class FileDecodeThread : public DecodeThread
{
    Q_OBJECT

public:
    FileDecodeThread(PageCache *pageCache);
    ~FileDecodeThread();

private:
    void decode(int pageNum, const QString &path, QSize fullSize, const QSize &boundingSize);
};

#endif
