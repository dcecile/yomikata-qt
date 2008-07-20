#ifndef ARTIFICER_H
#define ARTIFICER_H

#include <QThread>
#include <QImage>

#include "steward.h"

class Artificer : public QThread
{
    Q_OBJECT

    public:
        Artificer(QObject *parent, Source &source, Steward &steward);
        ~Artificer();

    private:
        void run();

    private:
        Source &_source;
        Steward &_steward;
        bool aborted;
};

#endif
