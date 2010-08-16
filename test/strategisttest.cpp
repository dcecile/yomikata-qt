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

#define SLACK(w, h) \
    do \
    { \
        QCOMPARE(_strategist.pageLayout().slack, QSize((w), (h))); \
    } \
    while (0)

#define METRICS(x0, y0, w0, h0, x1, y1, w1, h1, slackW, slackH) \
    do \
    { \
        DisplayMetrics temp = _strategist.pageLayout();\
        QCOMPARE(temp.pages[0], QRect((x0), (y0), (w0), (h0))); \
        QCOMPARE(temp.pages[1], QRect((x1), (y1), (w1), (h1))); \
        QCOMPARE(temp.slack, QSize((slackW), (slackH))); \
    } \
    while (0)

StrategistTest::StrategistTest(QObject *parent)
    : QObject(parent),  _book(), _strategist(_book)
{
    _book.reset(3);
    _strategist.reset();
}


StrategistTest::~StrategistTest()
{
}

void StrategistTest::simple()
{
    _strategist.setViewport(QSize(100, 50), QSize(100, 50));

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

    // Dual (wide)
    SET(2, 200, 80);
    FIT(2, 0, 5, 100, 40);

    // Dual (tall)
    SET(2, 160, 100);
    FIT(2, 10, 0, 80, 50);

    // Rounding two
    SET(0, 85, 100);
    SET(1, 85, 100);
    FIT(1, 8, 0, 42, 50);
    FIT(0, 50, 0, 42, 50);

    // Non-rounding dual
    SET(2, 170, 100);
    FIT(2, 8, 0, 85, 50);
}

void StrategistTest::zoom()
{
    _strategist.setViewport(QSize(200, 100), QSize(100, 50));

    // Two identical
    SET(0, 160, 200);
    SET(1, 160, 200);
    FIT(1, 0, 0, 80, 100);
    FIT(0, 80, 0, 80, 100);
    SLACK(60, 50);

    // Width inside
    SET(0, 80, 200);
    SET(1, 80, 200);
    FIT(1, 10, 0, 40, 100);
    FIT(0, 50, 0, 40, 100);
    SLACK(0, 50);

    // Height inside
    _strategist.setViewport(QSize(100, 200), QSize(50, 100));
    SET(0, 100, 120);
    SET(1, 100, 120);
    FIT(1, 0, 20, 50, 60);
    FIT(0, 50, 20, 50, 60);
    SLACK(50, 0);
    _strategist.setViewport(QSize(200, 100), QSize(100, 50));

    // Rounding two
    SET(0, 165, 200);
    SET(1, 165, 200);
    FIT(1, 0, 0, 82, 100);
    FIT(0, 82, 0, 82, 100);
    SLACK(64, 50);

    // Non-rounding dual
    _book.next();
    SET(2, 330, 200);
    FIT(2, 0, 0, 165, 100);
    SLACK(65, 50);
    _book.previous();

    // Rounding two, no slack
    SET(0, 85, 200);
    SET(1, 85, 200);
    FIT(1, 8, 0, 42, 100);
    FIT(0, 50, 0, 42, 100);
    SLACK(0, 50);
}

void StrategistTest::metrics()
{
    // Regular
    _strategist.setViewport(QSize(100, 50), QSize(100, 50));
    SET(0, 140, 200);
    SET(1, 80, 100);
    METRICS(53, 0, 35, 50, 13, 0, 40, 50, 0, 0);

    // Zoomed
    _strategist.setViewport(QSize(200, 100), QSize(100, 50));
    SET(0, 165, 200);
    SET(1, 165, 200);
    METRICS(82, 0, 82, 100, 0, 0, 82, 100, 64, 50);
}

#include "strategisttest.moc"
