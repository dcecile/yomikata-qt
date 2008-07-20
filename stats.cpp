#include "stats.h"

#include <KDebug>

Stats::Stats(QObject *parent)
    : QObject(parent)
{
    _responseTime = 0.0;
    _responseTimeHist = 0;
    _resizeTime = 0.0;
    _resizeTimeHist = 0;
}


Stats::~Stats()
{
    kDebug()<<"Response time:"<<QString::number(_responseTime/1000, 'g', 3);
    kDebug()<<"Resize time:"<<QString::number(_resizeTime/1000, 'g', 3);
}

void Stats::addResponseTime(int time)
{
    _responseTime = (_responseTime * _responseTimeHist + time)
                    / (_responseTimeHist + 1);
    if (_responseTimeHist + 1 <= MAX_HIST) {
        _responseTimeHist++;
    }
    kDebug()<<"Response time:"<<QString::number(_responseTime/1000, 'g', 3);
}

void Stats::addResizeTime(int time)
{
    _resizeTime = (_resizeTime * _resizeTimeHist + time)
                    / (_resizeTimeHist + 1);
    if (_resizeTimeHist + 1 <= MAX_HIST) {
        _resizeTimeHist++;
    }
    //kDebug()<<"Resize time:"<<QString::number(double(_resizeTime)/1000, 'g', 3);
    kDebug()<<"Resize time"<<time<<"ms";
}

#include "stats.moc"
