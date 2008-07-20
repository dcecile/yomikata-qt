#ifndef PAGETURNER_H
#define PAGETURNER_H

#include <QObject>

class PageTurner : public QObject
{
    Q_OBJECT

public:
    PageTurner();

public slots:
    void setTwoPageMode(bool enabled);

    void initialize(int startPageNum, int numPages);

private:
    bool _twoPageMode;

    int _numPages;

    int _primaryPage;
    int _targetPage[2];
};

#endif
