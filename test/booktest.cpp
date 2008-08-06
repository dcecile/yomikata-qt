#include "booktest.h"

#include <QTest>

#include "book.h"

#define PAIRED(page, pair) \
    do \
    { \
        QCOMPARE(_book->pairedPage((page)), (pair)); \
    } \
    while (0)

#define CURRENT(pageA, pageB) \
    do \
    { \
        QCOMPARE(_book->page0(), (pageA)); \
        QCOMPARE(_book->page1(), (pageB)); \
    } \
    while (0)

BookTest::BookTest()
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
    _book = new Book(5);

    // Each page is paired except the last
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, 3);
    PAIRED(3, 2);
    PAIRED(4, -1);

    delete _book;

    // Check for even pages book
    _book = new Book(6);

    // Each page is paired
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, 3);
    PAIRED(3, 2);
    PAIRED(4, 5);
    PAIRED(5, 4);

    delete _book;
    _book = NULL;
}

/**
 * Test that dual pages affect the pairings.
 */
void BookTest::dualPages()
{
    _book = new Book(7);

    // Set one dual, the others form pairs around it
    _book->setDual(2);
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, -1);
    PAIRED(3, 4);
    PAIRED(4, 3);
    PAIRED(5, 6);
    PAIRED(6, 5);

    // Set two more paired, some pairs can't form
    _book->setDual(1);
    _book->setDual(6);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, 4);
    PAIRED(4, 3);
    PAIRED(5, -1);
    PAIRED(6, -1);

    delete _book;
    _book = NULL;
}

/**
 * Test that basic page turning works.
 */
void BookTest::turning()
{
    _book = new Book(5);

    // Page to the end
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);
    _book->next();
    CURRENT(4, -1);

    // And back
    _book->previous();
    CURRENT(2, 3);
    _book->previous();
    CURRENT(0, 1);

    delete _book;
    _book = NULL;
}

/**
 * Test that basic page turning is affected by dual pages.
 */
void BookTest::turningWithDual()
{
    _book = new Book(5);

    // Check start
    CURRENT(0, 1);

    // Set a page dual
    _book->setDual(1);

    // Page to the end
    CURRENT(0, -1);
    _book->next();
    CURRENT(1, -1);
    _book->next();
    CURRENT(2, 3);
    _book->setDual(2);
    CURRENT(2, -1);
    _book->next();
    CURRENT(3, 4);

    // And back
    _book->previous();
    CURRENT(2, -1);
    _book->previous();
    CURRENT(1, -1);
    _book->previous();
    CURRENT(0, -1);

    delete _book;
    _book = NULL;
}

/**
 * Test that page turning will work with dual pages further on.
 */
void BookTest::dualFuture()
{
    // Use a big book
    _book = new Book(12);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);

    // Learn about a dual page
    _book->setDual(8);

    // Page forward and hit the dual page
    _book->next();
    CURRENT(4, 5);
    _book->next();
    CURRENT(6, 7);
    _book->next();
    CURRENT(8, -1);
    _book->next();
    CURRENT(9, 10);
    _book->next();
    CURRENT(11, -1);

    delete _book;

    // Use a big book
    _book = new Book(12);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);

    // Learn about a new dual page
    _book->setDual(7);

    // Page forward and hit the dual page, the preceding pair can't form
    _book->next();
    CURRENT(4, 5);
    _book->next();
    CURRENT(6, -1);
    _book->next();
    CURRENT(7, -1);
    _book->next();
    CURRENT(8, 9);
    _book->next();
    CURRENT(10, 11);

    delete _book;
    _book = NULL;
}

/**
 * Test that page turning will work with dual pages back further.
 */
void BookTest::dualPast()
{
    // Use a big book
    _book = new Book(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);

    // Learn about a dual page from before
    _book->setDual(0);

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
    _book->next();
    CURRENT(4, 5);

    // Reset
    _book->setDual(4);
    CURRENT(4, -1);

    // Page forward a bit
    _book->next();
    CURRENT(5, 6);
    _book->next();
    CURRENT(7, 8);

    // Learn about a dual page from before
    _book->setDual(6);

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
    _book->next();
    CURRENT(9, 10);

    delete _book;
    _book = NULL;
}

/**
 * Test that a page of the current pair being set dual will keep the correct
current page.
 */
void BookTest::currentDual()
{
    _book = new Book(7);

    // Test paging forward
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);
    _book->setDual(2);
    CURRENT(2, -1);
    _book->next();
    CURRENT(3, 4);
    _book->setDual(4);
    CURRENT(3, -1);
    _book->next();
    CURRENT(4, -1);
    _book->next();
    CURRENT(5, 6);

    delete _book;

    _book = new Book(11);

    // Page forward
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);
    _book->setDual(2);
    CURRENT(2, -1);
    _book->next();
    CURRENT(3, 4);
    _book->next();
    CURRENT(5, 6);
    _book->next();
    CURRENT(7, 8);
    _book->next();
    CURRENT(9, 10);

    // Test paging back, and pairs still start after dual pages
    _book->previous();
    CURRENT(7, 8);
    _book->setDual(8);
    CURRENT(8, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
    _book->previous();
    CURRENT(7, -1);
    _book->previous();
    CURRENT(5, 6);
    _book->setDual(5); // Collapsed a section to size 2
    CURRENT(6, 7);
    PAIRED(6, 7);
    _book->previous();
    CURRENT(5, -1);
    _book->previous();
    CURRENT(3, 4);
    _book->previous();
    CURRENT(2, -1);
    _book->previous();
    CURRENT(0, 1);
    _book->setDual(0);
    CURRENT(1, -1);
    _book->previous();
    CURRENT(0, -1);

    delete _book;
    _book = NULL;
}

/**
 * Test that page parity is kept consistent.
 */
void BookTest::persistentParity()
{
    // Use a big book
    _book = new Book(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);
    _book->next();
    CURRENT(4, 5);
    _book->next();
    CURRENT(6, 7);

    // Learn about a dual page from before
    _book->setDual(4);

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
    _book->previous();
    _book->previous();
    _book->previous();
    _book->previous();
    CURRENT(0, 1);

    // Past even page doesn't change parity
    _book->setDual(1);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, 3);
    PAIRED(4, -1);
    PAIRED(5, -1);
    PAIRED(6, 7);
    PAIRED(8, 9);
    PAIRED(10, -1);

    // Past odd page doesn't change parity
    _book->setDual(2);
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
    _book->setDual(8);
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
    _book->setDual(7);
    PAIRED(0, -1);
    PAIRED(1, -1);
    PAIRED(2, -1);
    PAIRED(3, -1);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);

    delete _book;
    _book = NULL;
}

/**
 * Test that pages cannot be stranded. There are 3 cases:
 * - a new dual page compresses the section coming before it
 * - a new dual page compresses the section coming before it and the current
 *   pages get changed
 * - paging backward, a dual page is learned to be the first page of the pair,
 *   and the following section is compressed to a size of two, changing the
 *   current pages
 */
void BookTest::neverStranded()
{
    // Use a big book
    _book = new Book(15);

    // Get off-set
    _book->next();
    _book->previous();
    _book->setDual(0);
    CURRENT(1, -1);

    // Collapse the section
    _book->setDual(3);
    CURRENT(1, 2);
    PAIRED(0, -1);
    PAIRED(1, 2);
    PAIRED(3, -1);
    PAIRED(4, 5);

    // Get off-set again
    _book->next();
    _book->next();
    _book->next();
    _book->previous();
    _book->setDual(4);
    CURRENT(5, -1);

    // Move way forward
    _book->next();
    _book->next();
    CURRENT(8, 9);

    // Collapse the section
    _book->setDual(7);
    CURRENT(8, 9);
    PAIRED(4, -1);
    PAIRED(5, 6);
    PAIRED(7, -1);

    // Set up a length three section
    _book->setDual(11);

    // Move to the start of the section after paging backward
    _book->next();
    _book->previous();
    CURRENT(8, 9);

    // Collapse the section
    _book->setDual(8);
    CURRENT(9, 10);
    PAIRED(7, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
    PAIRED(11, -1);
    PAIRED(12, 13);
    PAIRED(14, -1);

    /// @todo test double-stranding

    delete _book;
    _book = NULL;
}

/**
 * Test that shifting forward works.
 */

/**
 * Test that past shifts are remembered.
 */

#include "booktest.moc"
