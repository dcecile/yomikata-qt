#include "lister.h"

Lister::Lister(QObject *parent)
    : QObject(parent)
{
}

Lister::~Lister()
{
}

void Lister::finishListing(const QStringList &files, const vector<int> &fileSizes, QString startPageName)
{
    Q_ASSERT(files.size() == (int)fileSizes.size());

    kDebug()<<"File listing complete:";

    // Clear the old list
    _files.clear();

    // Store the data
    QStringList::iterator i;
    vector<int>::iterator j;
    FileInfo temp;
    for (i = files.begin(), j = fileSizes.begin(); i != files.end(); i++, j++) {
        //kDebug()<<*i<<*j;
        temp.name = *i;
        temp.size = *j;
        _files.push_back(temp);
    }

    // Sort the entries lexigraphically
    /// @todo Use std::sort, make sure directory trees get sorted correctly
    vector<FileInfo>::iterator k, l;
    for (k = _files.begin(); k != _files.end(); k++) {
        for (l = k + 1; l != _files.end(); l++) {
            if (QString::localeAwareCompare(k->name, l->name) > 0) {
                temp = *l;
                *l = *k;
                *k = temp;
            }
        }
    }

    // Find the first page

    // Deconstruct the lister
    delete _lister;
    _lister = NULL;

    // Notify that populating is done
    emit donePopulating(_files);
}

#include "lister.moc"
