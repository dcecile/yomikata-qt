#ifndef FILELIST_H
#define FILELIST_H

#include <QObject>
#include <QSize>

#include <vector>

#include "fileinfo.h"

class Source;
class Lister;

using std::vector;

/**
 * @defgroup mod_filelist File List module
 * Manages the file list.
 */

/**
 * @ingroup mod_filelist
 * @brief Contains a list of files
 *
 * Poplulates itself.
 *
 * Stores file names, dimensions, and compressed/uncompressed file size.
 */
class FileList : public QObject
{
    Q_OBJECT

public:
    FileList(Source &source, QObject *parent);

    void retrieve();

    ~FileList();

signals:
    void donePopulating(const vector<FileInfo> &files);

private slots:
    void doneListing(QStringList files, vector<int> fileSizes, QString startPageName);

private:
    Source &_source;
    Lister *_lister;

    vector<FileInfo> _files;
};

#endif
