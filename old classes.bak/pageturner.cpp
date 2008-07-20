#include <KDebug>

#include "pageturner.h"

PageTurner::PageTurner()
{
}

void setTwoPageMode(bool enabled)
{
}

void initialize(int startPageNum, int numPages)
{
}

void PageTurner::changePage()
{

    // Notify the page cache that these pages need displaying
    emit pagesChanged(_targetPage, _primaryPage);
}

#include "pageturner.moc"
