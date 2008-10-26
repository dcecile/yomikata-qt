#ifndef STRATEGIST_H
#define STRATEGIST_H

#include <QObject>

#include <QRect>

class Book;

class Strategist : public QObject
{
    Q_OBJECT

public:
    Strategist(Book &book, QObject *parent = NULL);
    ~Strategist();

    void reset();

    QRect pageLayout(int index);

    bool isFullPageSizeKnown(int index);
    void setFullPageSize(int index, QSize size);

    void setViewport(const QSize &size);
    QSize viewport() const;

private:
    Book &_book;
    QSize _viewport;
};

#endif
