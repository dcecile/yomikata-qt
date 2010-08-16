#ifndef STRATEGISTTEST_H
#define STRATEGISTTEST_H

#include <QObject>

#include "book.h"
#include "strategist.h"

class StrategistTest : public QObject
{
    Q_OBJECT

public:
    StrategistTest(QObject *parent = 0);
    ~StrategistTest();

private slots:
    void simple();
    void zoom();
    void metrics();

private:
    Book _book;
    Strategist _strategist;
};

#endif
