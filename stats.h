#ifndef STATS_H
#define STATS_H

#include <QObject>
#include <QSize>

#include <vector>

using std::vector;

/**
 * @defgroup mod_stats Stats module
 *
 */

/**
 * @ingroup mod_stats
 * @brief Keeps track of decoding times
 */
class Stats : public QObject
{
    Q_OBJECT

public:
    Stats(QObject *parent);

    void addResponseTime(int time);
    void addResizeTime(int time);

    ~Stats();

private:
    static const int MAX_HIST = 5;

private:
    double _responseTime;
    int _responseTimeHist;

    double _resizeTime;
    int _resizeTimeHist;
};

#endif
