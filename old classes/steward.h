#ifndef STEWARD_H
#define STEWARD_H

#include <QObject>
#include <QMutex>

#include "book.h"
#include "projector.h"

class Steward : public QObject
{
    Q_OBJECT

    public:
        Steward(QObject *parent, Book &book, Projector &projector);
        ~Steward();

        void getPageToDecode(int *pageNumber, QString *filename);

    private slots:
        void pageTurned();

    private:
        Book &_book;
        Projector &_projector;

        QMutex _mutex;
};

#endif
