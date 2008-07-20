#include "filelist.h"

#include <QStringList>
#include <KDebug>

#include "source.h"
#include "lister.h"

FileList::FileList(Source &source, QObject *parent)
    : QObject(parent), _source(source), _lister(NULL)
{
}


FileList::~FileList()
{
}

void FileList::retrieve()
{
    // Create a lister
    _lister = _source.createLister();

    // Connect to it
    connect(_lister, SIGNAL(doneListing(QStringList, vector<int>, QString)), this, SLOT(doneListing(QStringList, vector<int>, QString)));

    // Start the listing
    _lister->list();
}

void FileList::doneListing(QStringList files, vector<int> fileSizes, QString)
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

    // Notify Oyabun that populating is done
    emit donePopulating(_files);
}

#include "filelist.moc"
