#include "strategist.h"

#include "book.h"

Strategist::Strategist(Book &book, QObject *parent)
    : QObject(parent), _book(book)
{
}

Strategist::~Strategist()
{
}

void Strategist::reset()
{
}

QRect Strategist::pageLayout(int indexer)
{
    QSize size(_viewport.width() / 2, _viewport.height());

    // Offset the right page
    if (_book.pairedPageOffset(indexer) == -1)
    {
        return QRect(_viewport.width() / 2, 0, _viewport.width() / 2, _viewport.height());
    }
    // Put the left in the top left
    else
    {
        return QRect(0, 0, _viewport.width() / 2, _viewport.height());
    }
}

void Strategist::setFullPageSize(QSize size)
{
}

void Strategist::setViewport(const QSize &size)
{
    _viewport = size;
}

QSize Strategist::viewport() const
{
    return _viewport;
}

#include "strategist.moc"
