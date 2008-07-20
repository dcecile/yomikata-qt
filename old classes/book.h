#ifndef BOOK_H
#define BOOK_H

#include <QObject>
#include <QList>

#include <vector>

#include "fileinfo.h"

using std::vector;

class Book : public QObject
{
    Q_OBJECT

public:
    Book(QObject *parent);
    ~Book();

    void open(const Source &source);

    int getPageOne() const;
    int getPageTwo() const;

    QString getFilename(int pageNumber) const;
    int getFilesize(int pageNumber) const;
    QSize getImageSize(int pageNumber) const;
    int getPairedPage(int pageNumber) const;

    void setPageDouble(int pageNumber);
    void setPageNotDouble(int pageNumber);

public slots:
    void turnPageForward();
    void turnPageBackward();
    void turnPageForwardOnePage();
    void turnPageToStart();
    void turnPageToEnd();

signals:
    void forwardEnabled(bool enabled);
    void backwardEnabled(bool enabled);
    void pageTurned();
    void pagePairsChanged(int lowerPageNumber, int upperPageNumber);

private:
    void finishTurningPage();

private slots:
    void doneListing(const vector<FileInfo> &files);

private:
    bool _twoPageMode;

    Lister *_lister;

    int _numPages;
    int _primaryPage;
    int _targetPage[2];

    bool _forwardEnabled;
    bool _backwardEnabled;

    struct Page {
        Page(const FileInfo &file)
            : filesize(file.size), prefDouble(-1), nextDouble(-1), filename(file.name) {}

        int filesize;
        int prevDouble;
        int nextDouble;
        QString filename;
        QSize imageSize;
    };

    vector<Page> _pages;
};

#endif
