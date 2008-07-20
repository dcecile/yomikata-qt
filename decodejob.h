#ifndef DECODEJOB_H
#define DECODEJOB_H

#include <QImage>
#include <KUrl>
#include <threadweaver/Job.h>

class DecodeJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    DecodeJob(int pageNum, const QString &path, const QSize &boundingSize);

    int pageNum() const;
    QImage &image();
    const QSize &fullImageSize() const;

protected:
    const QString &path() const;
    const QSize &boundingSize() const;
    void setImage(const QImage &image);
    void setFullImageSize(const QSize &size);

private:
    int _pageNum;
    QString _path;
    QSize _boundingSize;
    QImage _image;
    QSize _fullImageSize;
};

#endif
