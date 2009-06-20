#include "mean.h"

#include <QDebug>

Mean::Mean(int saturation, int restart)
{
    _saturation = saturation;
    _restart = restart;
    _samples = _restart - 1;
}

Mean::~Mean()
{
}

void Mean::addSample(double value)
{
    _samples++;

    if (_samples == _restart)
    {
        _mean = value;
        _samples = 1;
        qDebug()<<"FPS"<<1000.0 / _mean;
    }
    else if (_samples < _saturation)
    {
        _mean = (_mean * (_samples - 1) + value) / _samples;
    }
    else
    {
        _mean = (_mean * (_saturation - 1) + value) / _saturation;
    }
}

double Mean::value() const
{
    return _mean;
}
