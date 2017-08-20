#include "book.h"

#include <QDebug>
#include <QMutex>

Book::Book(QObject *parent)
    : QObject(parent)
{
    reset(0);
}

void Book::reset(int numPages)
{
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

    _importantPageIsFirst = true;

    // Set resize the info list
    _info.resize(_numPages);

    // Set the dual info
    for (int i = 0; i < _numPages; i++)
    {
        _info[i].dual = false;
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

    // Notify changed
    emit changed();
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
    _importantPageIsFirst = true;

    // Notify changed
    emit changed();
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
    _importantPageIsFirst = false;

    // Notify changed
    emit changed();
}

void Book::setPage(int page)
{
    Q_ASSERT(0 <= page && page < _numPages);
    //qDebug()<<"Start"<<page<<_page0<<_page1;

    // Go forward while the target is after the first and not at the second
    while (page > _page0 && page != _page1)
    {
        next();
    }

    // Go backward while the target is before the first
    while (page < _page0)
    {
        previous();
    }

    // Should be at the target now
    //qDebug()<<"End"<<page<<_page0<<_page1;
    Q_ASSERT(_page0 == page || _page1 == page);

    // Notify changed
    emit changed();
}

/**
 * A shift only happens if currently displaying one or two nondual pages and not
 * at the end of a section; otherwise it is a normal page turn. When actually
 * shifting, modify the parity of all pages in the section and make the second
 * page the important one since it hasn't been seen yet.
 */
void Book::shiftNext()
{
    Q_ASSERT(_page0 < _numPages - 1 && _page1 < _numPages - 1);

    bool shift = true;

    // Check that not displaying a dual page
    if (_info[_page0].dual)
    {
        shift = false;
    }

    // Figure out the page to shift from
    // (Special case when viewing the solitary page at the start of a section)
    int lastPage;

    if (_page1 == -1)
    {
        lastPage = _page0;
    }
    else
    {
        lastPage = _page1;
    }

    // Check that the next page isn't dual
    if (_info[lastPage + 1].dual)
    {
        shift = false;
    }

    // If checks failed, just do a regular page turn
    if (!shift)
    {
        next();
        return;
    }

    // Set the curent pages to be shifted
    _page0 = lastPage;
    _page1 = lastPage + 1;

    // The important page is the second (not see yet) page
    _importantPageIsFirst = false;

    // Find the start of this section
    int boundary;

    for (boundary = _page0 - 1; boundary >= 0; boundary--)
    {
        if (_info[boundary].dual)
        {
            break;
        }
    }

    // Reset all pairs until the next dual page

    // If the section starts at a page that would be paired to the previous
    // page, skip over it
    int i = boundary + 1;

    if (i % 2 == _page1 % 2)
    {
        _info[i].pair = None;
        i++;
    }

    // Continue with all pairs until the section end
    for (; i < _numPages - 1
         && !_info[i].dual && !_info[i + 1].dual; i += 2)
    {
        _info[i].pair = Next;
        _info[i + 1].pair = Previous;
    }

    // If section ends in a solitary page, account for it
    if (i < _numPages && !_info[i].dual)
    {
        _info[i].pair = None;
    }

    // Notify changed
    emit changed();
}

/**
 * Set a page as dual. To maintain how pairs should start after dual pages,
 * and the current parity never changes, the following procedure is used.
 * - unpair the dual page
 * - set the current page correctly
 * - if parity needs changing and the current parity won't change
 *     - reset the parities after the new dual page, until another dual page is
 *       found, or the end of the book is reached
 * - keep pages from being stranded in the three cases
 *     - a new dual page compresses the section coming before it
 *     - a new dual page compresses the section coming before it and the current
 *       pages get changed
 *     - paging backward, a dual page is learned to be the first page of the
 *       pair, and the following section is compressed to a size of two,
 *       changing the current pages
 */
void Book::setDual(int page)
{
    Q_ASSERT(page >= 0 && page < _numPages);

    int initialPage0 = _page0;
    int initialPage1 = _page1;

    // If the page was paired, unpair it
    if (_info[page].pair == Previous)
    {
        _info[page - 1].pair = None;
        _info[page].pair = None;
    }
    else if (_info[page].pair == Next)
    {
        _info[page + 1].pair = None;
        _info[page].pair = None;
    }

    // Set the page as dual
    _info[page].dual = true;

    // If hit a page of the current two, switch to single
    if (_page1 != -1 && (page == _page0 || page == _page1))
    {
        // If going forward, drop the second page
        if (_importantPageIsFirst)
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

    // Check if parity needs changing
    bool changeNeeded = false;

    // The page needs to be before the second last page
    // And the next two pages need to be non-dual and unpaired
    if (page < _numPages - 2 && !_info[page + 1].dual && !_info[page + 2].dual
            && _info[page + 1].pair == None)
    {
        changeNeeded = true;
    }

    // Check if the current parity would be changed
    // If the current page comes after the dual page and there are no dual pages
    // between them, cancel the change
    if (changeNeeded && _page0 > page)
    {
        bool interveningDualPage = false;

        for (int i = page + 1; i < _page0; i++)
        {
            if (_info[i].dual)
            {
                interveningDualPage = true;
            }
        }

        if (!interveningDualPage)
        {
            changeNeeded = false;
        }
    }

    // If the change is needed, reset the parity for the pages after the new
    // dual page
    if (changeNeeded)
    {
        // Keep reseting pairs until a dual page or the end of the book is
        // reached
        int i;

        for (i = page + 1; i < _numPages - 1 && !_info[i].dual
                && !_info[i + 1].dual; i += 2)
        {
            _info[i].pair = Next;
            _info[i + 1].pair = Previous;
        }

        // Make sure the last page in the sequence is solitary (it's dual or
        // the next is dual, or it's the last page)
        if (i < _numPages && !_info[i].dual)
        {
            _info[i].pair = None;
        }
    }

    // Test for stranding behind
    if (page > 1)
    {
        bool solitarySingle0 = !_info[page - 2].dual
                && _info[page - 2].pair == None;
        bool solitarySingle1 = !_info[page - 1].dual
                && _info[page - 1].pair == None;

        if (solitarySingle0 && solitarySingle1)
        {
            // The preceding page should be dual
            Q_ASSERT(page == 2 || _info[page - 3].dual);

            // Pair the two stranded pages
            _info[page - 2].pair = Next;
            _info[page - 1].pair = Previous;

            // Check if the current pages need pairing
            if (_page0 == page - 2 || _page0 == page - 1)
            {
                Q_ASSERT(_page1 == -1);
                _page0 = page - 2;
                _page1 = page -1;
            }
        }
    }

    // Test for stranding forward
    if (!_importantPageIsFirst &&_page0 == page + 1 && page < _numPages - 2)
    {
        bool solitarySingle0 = !_info[_page0].dual && _page1 == -1;
        bool solitarySingle1 = !_info[page + 2].dual
                && _info[page + 2].pair == None;

        if (solitarySingle0 && solitarySingle1)
        {
            // The proceding page should be dual
            Q_ASSERT(page == _numPages - 3 || _info[page + 3].dual);

            // Pair the two stranded pages
            _page1 = _page0 + 1;
            _info[_page0].pair = Next;
            _info[_page1].pair = Previous;
        }
    }

    // Notify changed
    emit changed();

    // Notify if the current pages changed
    if (initialPage0 != _page0 || initialPage1 != _page1)
    {
        emit dualCausedPageChange();
    }
}

bool Book::isDual(int page)
{
    Q_ASSERT(page >= 0 && page < _numPages);
    return _info[page].dual;
}

int Book::page0()
{
    return _page0;
}

int Book::page1()
{
    return _page1;
}

int Book::pairedPage(int page)
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

int Book::pairedPageOffset(int page)
{
    Q_ASSERT(page >= 0 && page < _numPages);

    return (int) _info[page].pair;
}

int Book::numPages()
{
    return _numPages;
}

bool Book::isNextEnabled()
{
    return _page0 < _numPages - 1 && _page1 < _numPages - 1;
}

bool Book::isPreviousEnabled()
{
    return _page0 > 0;
}
