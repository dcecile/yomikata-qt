#ifndef PAGE_H
#define PAGE_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QPixmap>

#include "fileinfo.h"

class Planner;
class Cache;

class Page : public QObject
{
    Q_OBJECT

public:
    Page(int pageNumber, const FileInfo &fileInfo, Planner &planner, Cache &cache, QObject *parent);

    int getPageNumber() const;
    const QString &getFilename() const;

    bool isFullImageSizeKnown();
    void setFullImageSize(QSize size);
    QSize getTargetSize();
    QRect getTargetRect();

    bool isCached();
    void setPixmap(const QPixmap &pixmap);
    QPixmap getPixmap();
    void setThumbnail(const QPixmap &thumbnail);
    QPixmap getThumbnail();

    bool isDecodeNeeded();
    void decodeStarted();
    void decodeAborted();
    void decodeSizeSet(QSize size);

    ~Page();

signals:
    void pixmapUpdated(Page *page);

public:
    static const QSize THUMBNAIL_SIZE;

private:
    Planner &_planner;
    Cache &_cache;

    int _pageNumber;
    QPixmap _thumbnail;
};

#endif
