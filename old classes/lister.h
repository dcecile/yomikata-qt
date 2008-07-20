#ifndef LISTER_H
#define LISTER_H

#include <QObject>
#include <QStringList>

#include <vector>

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
    virtual void list() = 0;

    virtual ~Lister();

signals:
    void doneListing(QStringList files, vector<int> fileSizes, QString startPageName);
};

#endif
