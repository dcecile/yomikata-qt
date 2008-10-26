#ifndef ARTIFICER_H
#define ARTIFICER_H

#include <QObject>

#include <QPixmap>

class Strategist;

class Artificer : public QObject
{
    Q_OBJECT

public:
    Artificer(Strategist &strategist, QObject *parent = NULL);
    ~Artificer();

    void reset();

    void decodePage(int indexer);

signals:
    void pageDecoded(int indexer, QPixmap page);

private:
    Strategist &_strategist;
};

#endif
