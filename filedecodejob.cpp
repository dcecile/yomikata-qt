#include <KDebug>

#include "filedecodejob.h"

FileDecodeJob::FileDecodeJob(int pageNum, KUrl path)
    :_pageNum(pageNum), _path(path)
{
}

int FileDecodeJob::pageNum() const
{
    return _pageNum;
}
QPixmap FileDecodeJob::pixmap()
{
    return _pixmap;
}
void FileDecodeJob::run()
{
    kDebug()<<"Decode job executing"<<endl
        <<"Local file of "<<_path.url()<<" is "<<_path.toLocalFile()<<endl;
    _pixmap = QPixmap(_path.toLocalFile());
}

#include "filedecodejob.moc"
