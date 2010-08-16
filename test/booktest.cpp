#include "booktest.h"

#include <QTest>

#define PAIRED(page, pair) \
    do \
    { \
        QCOMPARE(_book.pairedPage((page)), (pair)); \
    } \
    while (0)

#define CURRENT(pageA, pageB) \
    do \
    { \
        QCOMPARE(_book.page0(), (pageA)); \
        QCOMPARE(_book.page1(), (pageB)); \
    } \
    while (0)

BookTest::BookTest()
    : _book(this)
{
}

BookTest::~BookTest()
{
}

/**
 * Test simple pairing.
 */
void BookTest::simple()
{
    // Check for odd pages book
    _book.reset(5);

    // Each page is paired except the last
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, 3);
    PAIRED(3, 2);
    PAIRED(4, -1);

    // Check for even pages book
    _book.reset(6);

    // Each page is paired
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, 3);
    PAIRED(3, 2);
    PAIRED(4, 5);
    PAIRED(5, 4);

    // Test a one page book
    _book.reset(1);

    // No paired pages
    PAIRED(0, -1);
    CURRENT(0, -1);
}

/**
 * Test that dual pages affect the pairings.
 */
void BookTest::dualPages()
{
    _book.reset(7);

    // Set one dual, the others form pairs around it
    _book.setDual(2);
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, -1);
    PAIRED(3, 4);
    PAIRED(4, 3);
    PAIRED(5, 6);
    PAIRED(6, 5);

    // Set two more paired, some pairs can't form
    _book.setDual(1);
    _book.setDual(6);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, 4);
    PAIRED(4, 3);
    PAIRED(5, -1);
    PAIRED(6, -1);
}

/**
 * Test that basic page turning works.
 */
void BookTest::turning()
{
    _book.reset(5);

    // Page to the end
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);
    _book.next();
    CURRENT(4, -1);

    // And back
    _book.previous();
    CURRENT(2, 3);
    _book.previous();
    CURRENT(0, 1);
}

/**
 * Test that basic page turning is affected by dual pages.
 */
void BookTest::turningWithDual()
{
    _book.reset(5);

    // Check start
    CURRENT(0, 1);

    // Set a page dual
    _book.setDual(1);

    // Page to the end
    CURRENT(0, -1);
    _book.next();
    CURRENT(1, -1);
    _book.next();
    CURRENT(2, 3);
    _book.setDual(2);
    CURRENT(2, -1);
    _book.next();
    CURRENT(3, 4);

    // And back
    _book.previous();
    CURRENT(2, -1);
    _book.previous();
    CURRENT(1, -1);
    _book.previous();
    CURRENT(0, -1);
}

/**
 * Test that page turning will work with dual pages further on.
 */
void BookTest::dualFuture()
{
    // Use a big book
    _book.reset(12);

    // Page forward a bit
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);

    // Learn about a dual page
    _book.setDual(8);

    // Page forward and hit the dual page
    _book.next();
    CURRENT(4, 5);
    _book.next();
    CURRENT(6, 7);
    _book.next();
    CURRENT(8, -1);
    _book.next();
    CURRENT(9, 10);
    _book.next();
    CURRENT(11, -1);

    // Use a big book
    _book.reset(12);

    // Page forward a bit
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);

    // Learn about a new dual page
    _book.setDual(7);

    // Page forward and hit the dual page, the preceding pair can't form
    _book.next();
    CURRENT(4, 5);
    _book.next();
    CURRENT(6, -1);
    _book.next();
    CURRENT(7, -1);
    _book.next();
    CURRENT(8, 9);
    _book.next();
    CURRENT(10, 11);
}

/**
 * Test that page turning will work with dual pages back further.
 */
void BookTest::dualPast()
{
    // Use a big book
    _book.reset(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);

    // Learn about a dual page from before
    _book.setDual(0);

    // The current parity will not change
    CURRENT(2, 3);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, 3);
    PAIRED(4, 5);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Page forward
    _book.next();
    CURRENT(4, 5);

    // Reset
    _book.setDual(4);
    CURRENT(4, -1);

    // Page forward a bit
    _book.next();
    CURRENT(5, 6);
    _book.next();
    CURRENT(7, 8);

    // Learn about a dual page from before
    _book.setDual(6);

    // The current parity will not change
    CURRENT(7, 8);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, -1);
    PAIRED(7, 8);
    PAIRED(9, 10);

    // Page to the end
    _book.next();
    CURRENT(9, 10);

    // Use a medium book
    _book.reset(11);

    // Page forward a bit
    _book.next();
    _book.next();
    _book.next();
    CURRENT(6, 7);

    // Learn about a dual page from before
    _book.setDual(4);

    // The current parity will not change
    CURRENT(6, 7);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);

    // Set one from the previous section
    _book.setDual(0);

    // Other parity changes, not the current
    CURRENT(6, 7);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
}

/**
 * Test that a page of the current pair being set dual will keep the correct
current page.
 */
void BookTest::currentDual()
{
    _book.reset(7);

    // Test paging forward
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);
    _book.setDual(2);
    CURRENT(2, -1);
    _book.next();
    CURRENT(3, 4);
    _book.setDual(4);
    CURRENT(3, -1);
    _book.next();
    CURRENT(4, -1);
    _book.next();
    CURRENT(5, 6);

    _book.reset(11);

    // Page forward
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);
    _book.setDual(2);
    CURRENT(2, -1);
    _book.next();
    CURRENT(3, 4);
    _book.next();
    CURRENT(5, 6);
    _book.next();
    CURRENT(7, 8);
    _book.next();
    CURRENT(9, 10);

    // Test paging back, and pairs still start after dual pages
    _book.previous();
    CURRENT(7, 8);
    _book.setDual(8);
    CURRENT(8, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
    _book.previous();
    CURRENT(7, -1);
    _book.previous();
    CURRENT(5, 6);
    _book.setDual(5); // Collapsed a section to size 2
    CURRENT(6, 7);
    PAIRED(6, 7);
    _book.previous();
    CURRENT(5, -1);
    _book.previous();
    CURRENT(3, 4);
    _book.previous();
    CURRENT(2, -1);
    _book.previous();
    CURRENT(0, 1);
    _book.setDual(0);
    CURRENT(1, -1);
    _book.previous();
    CURRENT(0, -1);
}

/**
 * Test that page parity is kept consistent.
 */
void BookTest::persistentParity()
{
    // Use a big book
    _book.reset(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);
    _book.next();
    CURRENT(4, 5);
    _book.next();
    CURRENT(6, 7);

    // Learn about a dual page from before
    _book.setDual(4);

    // The current parity will not change
    CURRENT(6, 7);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Go back to the start of the book
    _book.previous();
    _book.previous();
    _book.previous();
    _book.previous();
    CURRENT(0, 1);

    // Past even page doesn't change parity
    _book.setDual(1);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Past odd page doesn't change parity
    _book.setDual(2);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Future even page doesn't change parity
    _book.setDual(8);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, -1);
    PAIRED(9, 10);

    // Future odd page changes parity because a section size is
    // reduced to two
    _book.setDual(7);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
}

/**
 * Test that pages cannot be stranded. There are 3 cases:
 * - a new dual page compresses the section coming before it
 * - a new dual page compresses the section coming before it and the current
 *   pages get changed
 * - paging backward, a dual page is learned to be the first page of the pair,
 *   and the following section is compressed to a size of two, changing the
 *   current pages
 *
 * Note: It is impossible to do two collapses at once. ("p p xp p")
 */
void BookTest::neverStranded()
{
    // Use a big book
    _book.reset(15);

    // Get off-set
    _book.next();
    _book.previous();
    _book.setDual(0);
    CURRENT(1, -1);

    // Collapse the section
    _book.setDual(3);
    CURRENT(1, 2);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, 5);

    // Get off-set again
    _book.next();
    _book.next();
    _book.next();
    _book.previous();
    _book.setDual(4);
    CURRENT(5, -1);

    // Move way forward
    _book.next();
    _book.next();
    CURRENT(8, 9);

    // Collapse the section
    _book.setDual(7);
    CURRENT(8, 9);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);

    // Set up a length three section
    _book.setDual(11);

    // Move to the start of the section after paging backward
    _book.next();
    _book.previous();
    CURRENT(8, 9);

    // Collapse the section
    _book.setDual(8);
    CURRENT(9, 10);
    PAIRED(7, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
    PAIRED(11, -1);
    PAIRED(12, 13);
    PAIRED(14, -1);
}

/**
 * Test that shifting forward works.
 */
void BookTest::shifting()
{
    // Simple book
    _book.reset(8);

    // Shift
    _book.shiftNext();
    CURRENT(1, 2);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, 4);
    PAIRED(5, 6);
    PAIRED(7, -1);

    // Shift
    _book.shiftNext();
    CURRENT(2, 3);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, 5);
    PAIRED(6, 7);

    // Shift
    _book.shiftNext();
    CURRENT(3, 4);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, 4);
    PAIRED(5, 6);
    PAIRED(7, -1);

    // Page back
    _book.previous();
    CURRENT(1, 2);
    _book.previous();
    CURRENT(0, -1);

    // Set a dual page
    _book.setDual(4);

    // Shift through it
    _book.shiftNext();
    CURRENT(0, 1);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    _book.shiftNext();
    CURRENT(1, 2);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    _book.shiftNext();
    CURRENT(2, 3);
    _book.shiftNext();
    CURRENT(4, -1);
    _book.shiftNext();
    CURRENT(5, 6);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    _book.shiftNext();
    CURRENT(6, 7);
    PAIRED(0, 1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
}
/**
 * Test that shifting always allows all page pairings to be seen.
 */
void BookTest::shiftingCoverage()
{
    // Size 3 book
    _book.reset(3);
    CURRENT(0, 1);

    // Shift
    _book.shiftNext();
    CURRENT(1, 2);

    // Go back
    _book.previous();
    CURRENT(0, -1);

    // Go forward
    _book.next();
    CURRENT(1, 2);

    // Go back, then shift
    _book.previous();
    _book.shiftNext();
    CURRENT(0, 1);

    // Go next
    _book.next();
    CURRENT(2, -1);

    // Size 4 book
    _book.reset(4);
    CURRENT(0, 1);

    // Shift
    _book.shiftNext();
    CURRENT(1, 2);

    // Go forward and back
    _book.next();
    CURRENT(3, -1);
    _book.previous();
    _book.previous();
    CURRENT(0, -1);

    // Go forward, then shift
    _book.next();
    CURRENT(1, 2);
    _book.shiftNext();
    CURRENT(2, 3);

    // Go back
    _book.previous();
    CURRENT(0, 1);
}

/**
 * Test that past shifts are remembered.
 */
void BookTest::currentDualWithShifting()
{
    // Simple book
    _book.reset(6);

    // Shift
    _book.shiftNext();
    CURRENT(1, 2);

    // Set other page dual
    _book.setDual(1);
    CURRENT(2, -1);

    // Shift forward
    _book.shiftNext();
    CURRENT(2, 3);

    // Set other page dual again
    _book.setDual(2);
    CURRENT(3, -1);

    // Shift forward
    _book.shiftNext();
    CURRENT(3, 4);

    // Set primary page dual
    _book.setDual(4);
    CURRENT(4, -1);
}

/**
 * Test that past shifts are remembered.
 */
void BookTest::persistentShiftedParity()
{
    // Use a big book
    _book.reset(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book.next();
    CURRENT(2, 3);
    _book.next();
    CURRENT(4, 5);
    _book.shiftNext();
    CURRENT(5, 6);

    // Learn about a dual page from before
    _book.setDual(4);

    // The current parity will not change
    CURRENT(5, 6);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, 8);
    PAIRED(9, 10);

    // Offset this section
    _book.shiftNext();
    CURRENT(6, 7);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Go back to the start of the book
    _book.previous();
    _book.previous();
    _book.previous();
    _book.previous();
    _book.shiftNext();
    _book.previous();
    CURRENT(0, 1);

    // Past even page doesn't change parity
    _book.setDual(1);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Past odd page doesn't change parity
    _book.setDual(2);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Future even page doesn't change parity
    _book.setDual(8);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, -1);
    PAIRED(9, 10);

    // Future odd page changes parity because a section size is
    // reduced to two
    _book.setDual(7);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
}

#include "booktest.moc"
