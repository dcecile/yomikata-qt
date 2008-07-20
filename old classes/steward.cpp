#include "steward.h"

Steward::Steward(QObject *parent, Book &book, Projector &projector)
    : QObject(parent), _book(book), _projector(projector)
{
    connect(&_book, SIGNAL(pageTurned()), this, SLOT(pageTurned()));
}

void Steward::getPageToDecode(int *pageNumber, QString *filename)
{
    QMutexLocker locker(&_mutex);
}

void Steward::pageTurned()
{
    QMutexLocker locker(&_mutex);
}