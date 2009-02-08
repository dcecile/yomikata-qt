#include "strategisttest.h"

#include <QTest>

#include "debug.h"

#define SET(page, w, h) \
    do \
    { \
        _strategist.setFullPageSize((page), QSize((w), (h))); \
    } \
    while (0)

#define FIT(page, x, y, w, h) \
    do \
    { \
        QCOMPARE(_strategist.pageLayout((page)), QRect((x), (y), (w), (h))); \
    } \
    while (0)

StrategistTest::StrategistTest(QObject *parent)
    : QObject(parent), _lock(QMutex::Recursive), _book(_lock), _strategist(_book, _lock)
{
    _strategist.setViewport(QSize(100, 50), QSize());
    _book.reset(2);
    _strategist.reset();
}


StrategistTest::~StrategistTest()
{
}

void StrategistTest::simple()
{
    // Two identical
    SET(0, 80, 100);
    SET(1, 80, 100);
    FIT(1, 10, 0, 40, 50);
    FIT(0, 50, 0, 40, 50);

    // Two different
    SET(0, 80, 100);
    SET(1, 140, 200);
    FIT(1, 13, 0, 35, 50);
    FIT(0, 48, 0, 40, 50);

    // Two different (swapped)
    SET(0, 140, 200);
    SET(1, 80, 100);
    FIT(1, 13, 0, 40, 50);
    FIT(0, 53, 0, 35, 50);

    // Two different (bigger wider)
    SET(0, 70, 100);
    SET(1, 160, 200);
    FIT(1, 13, 0, 40, 50);
    FIT(0, 53, 0, 35, 50);

    // Double (wide)
    SET(0, 200, 80);
    FIT(0, 0, 5, 100, 40);

    // Double (tall)
    SET(0, 160, 100);
    FIT(0, 10, 0, 80, 50);
}

#include "strategisttest.moc"
