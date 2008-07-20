#ifndef LISTER_H
#define LISTER_H

#include <QStringList>

class Lister : public QObject
{
    Q_OBJECT

public:
    Lister();

signals:
    void listBuilt(int initialPosition, const QStringList &files);
};

#endif
