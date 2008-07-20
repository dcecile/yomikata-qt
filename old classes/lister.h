#ifndef LISTER_H
#define LISTER_H

#include <QObject>
#include <QStringList>

#include <vector>

#include "fileinfo.h"

using std::vector;

/**
 * @ingroup mod_filelist
 * @brief Returns the list of files from the Source.
 */
class Lister : public QObject
{
    Q_OBJECT

protected:
    Lister(QObject *parent);

public:
    virtual void beginListing() = 0;

    virtual ~Lister();

protected:
    void finishListing(const QStringList &files, const vector<int> &fileSizes, QString startPageName);

signals:
    void doneListing(const vector<FileInfo> &files);
};

#endif
