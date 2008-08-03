#ifndef BOOKTEST_H
#define BOOKTEST_H

#include <QObject>

class Book;

class BookTest : public QObject
{
    Q_OBJECT

public:
    BookTest();
    ~BookTest();

private:
    void paired(int page, int paired);
    void current(int page0, int page1);

private slots:
    void simple();
    void widePages();
    void turning();
    void turningWithWide();
    void wideFuture();
    void widePast();
    void currentWide();

private:
    Book *_book;
};

#endif
