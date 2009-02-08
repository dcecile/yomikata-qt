#ifndef DISPLAYMETRICS_H
#define DISPLAYMETRICS_H

#include <QRect>

struct DisplayMetrics
{
public:
    QRect pages[2];
    QSize slack;
};

#endif
