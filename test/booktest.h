#ifndef BOOKTEST_H
#define BOOKTEST_H

#include <QObject>

#include <QMutex>

#include "book.h"

/**
 * @brief Unit testing for Book. Excercises all page turning and dual page
 * situations.
 */
class BookTest : public QObject
{
    Q_OBJECT

public:
    BookTest();
    ~BookTest();

private slots:
    void simple();
    void dualPages();
    void turning();
    void turningWithDual();
    void dualFuture();
    void dualPast();
    void currentDual();
    void persistentParity();
    void neverStranded();
    void shifting();
    void shiftingCoverage();
    void currentDualWithShifting();
    void persistentShiftedParity();

private:
    QMutex _lock;
    Book _book;
};

#endif
