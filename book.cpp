#include "book.h"

#include <QDebug>

Book::Book(int numPages)
{
    Q_ASSERT(numPages > 0);

    // Set the number of pages
    _numPages = numPages;

    // Set the current pages
    _page0 = 0;

    if (_numPages > 1)
    {
        _page1 = 1;
    }
    else
    {
        _page1 = -1;
    }

    _goingForward = true;

    // Set resize the info list
    _info.resize(_numPages);

    // Set the wide info
    for (int i = 0; i < _numPages; i++)
    {
        _info[i].wide = false;
    }

    // Initialize the pairings
    for (int i = 0; i < _numPages; i += 2)
    {
        _info[i].pair = Next;
    }

    for (int i = 1; i < _numPages; i += 2)
    {
        _info[i].pair = Previous;
    }

    if (_numPages % 2 == 1)
    {
        _info.back().pair = None;
    }
}

Book::~Book()
{
}

void Book::next()
{
    Q_ASSERT(_page0 < _numPages - 1 && _page1 < _numPages - 1);

    // Go to the one after the second page
    if (_page1 != -1)
    {
        _page0 = _page1 + 1;
    }
    // Or go to the next page
    else
    {
        _page0++;
    }

    // Set the second page to the pair
    _page1 = pairedPage(_page0);

    // Going forward
    _goingForward = true;
}

void Book::previous()
{
    Q_ASSERT(_page0 > 0);

    // Get the pair of the previous page
    int pair = pairedPage(_page0 - 1);

    // If there is a pair, set both pages
    if (pair != -1)
    {
        _page1 = _page0 - 1;
        _page0 = pair;
    }
    // Otherwise set only the first
    else
    {
        _page0--;
        _page1 = -1;
    }

    // Going backward
    _goingForward = false;
}

void Book::setWide(int page)
{
    Q_ASSERT(page >= 0 && page < _numPages);

    // Set the page as wide
    _info[page].wide = true;

    // If hit a page of the current two, switch to single
    if (_page1 != -1 && (page == _page0 || page == _page1))
    {
        // If going forward, drop the second page
        if (_goingForward)
        {
            _page1 = -1;
        }
        // If going backward, drop the first page
        else
        {
            _page0 = _page1;
            _page1 = -1;
        }
    }

    // Reset all of the pairs
    int i = 0;

    do
    {
        // If this is the last page, or it's wide, set it to solitary
        if (i == _numPages - 1 || _info[i].wide)
        {
            _info[i].pair = None;
            i++;
        }
        // If the next page is wide, set this and the next to solitary
        else if (_info[i + 1].wide)
        {
            _info[i].pair = None;
            _info[i + 1].pair = None;
            i += 2;
        }
        // Else, set as adjacent pages
        else
        {
            _info[i].pair = Next;
            _info[i + 1].pair = Previous;
            i += 2;
        }
    }
    while (i < _numPages);
}

int Book::page0() const
{
    return _page0;
}

int Book::page1() const
{
    return _page1;
}

int Book::pairedPage(int page) const
{
    Q_ASSERT(page >= 0 && page < _numPages);

    const Pair &pair = _info[page].pair;
    if (pair == Previous)
    {
        return page - 1;
    }
    else if (pair == None)
    {
        return -1;
    }
    else
    {
        return page + 1;
    }
}


