#include "artificer.h"

Artificer::Artificer(QObject *parent, Source &source, Steward &steward)
    : QObject(parent), _source(source),  _steward(steward)
{
    aborted = false;
}

Artificer::~Artificer()
{
    aborted = true;
}

void Artificer::run()
{
    int pageNumber;
    QString filename;

    while (!aborted) {
        _steward.getPageToDecode(&pageNumber, &filename);
    }
}