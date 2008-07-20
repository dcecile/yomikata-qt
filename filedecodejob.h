#ifndef FILEDECODEJOB_H
#define FILEDECODEJOB_H

#include <QPixmap>
#include <QImageReader>
#include <QIODevice>
#include <KUrl>
#include <threadweaver/Job.h>

class FileDecodeJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    FileDecodeJob(int pageNum, KUrl path);

    int pageNum() const;
    QPixmap pixmap();

private:
    void run();

private:
    int _pageNum;
    KUrl _path;
    QPixmap _pixmap;
};

#endif
