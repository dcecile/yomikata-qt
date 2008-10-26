#include "artificer.h"

Artificer::Artificer(Strategist &strategist, QObject *parent)
    : QObject(parent), _strategist(strategist)
{
}


Artificer::~Artificer()
{
}

void Artificer::reset()
{
}

void Artificer::decodePage(int indexer)
{
}

#include "artificer.moc"
