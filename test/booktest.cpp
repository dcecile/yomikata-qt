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
 * Test that wide pages affect the pairings.
 */
void BookTest::widePages()
{
    _book = new Book(7);

    // Set one wide, the others form pairs around it
    _book->setWide(2);
    PAIRED(0, 1);
    PAIRED(1, 0);
    PAIRED(2, -1);
    PAIRED(3, 4);
    PAIRED(4, 3);
    PAIRED(5, 6);
    PAIRED(6, 5);

    // Set two more paired, some pairs can't form
    _book->setWide(1);
    _book->setWide(6);
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
 * Test that basic page turning is affected by wide pages.
 */
void BookTest::turningWithWide()
{
    _book = new Book(5);

    // Check start
    CURRENT(0, 1);

    // Set a page wide
    _book->setWide(1);

    // Page to the end
    CURRENT(0, -1);
    _book->next();
    CURRENT(1, -1);
    _book->next();
    CURRENT(2, 3);
    _book->setWide(2);
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
 * Test that page turning will work with wide pages further on.
 */
void BookTest::wideFuture()
{
    // Use a big book
    _book = new Book(12);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);

    // Learn about a wide page
    _book->setWide(8);

    // Page forward and hit the wide page
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

    // Learn about a new wide page
    _book->setWide(7);

    // Page forward and hit the wide page, the preceding pair can't form
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
 * Test that page turning will work with wide pages back further.
 */
void BookTest::widePast()
{
    // Use a big book
    _book = new Book(11);

    // Page forward a bit
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);

    // Learn about a wide page from before
    _book->setWide(0);

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
    _book->setWide(4);
    CURRENT(4, -1);

    // Page forward a bit
    _book->next();
    CURRENT(5, 6);
    _book->next();
    CURRENT(7, 8);

    // Learn about a wide page from before
    _book->setWide(6);

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
 * Test that a page of the current pair being set wide will keep the correct
current page.
 */
void BookTest::currentWide()
{
    _book = new Book(7);

    // Test paging forward
    CURRENT(0, 1);
    _book->next();
    CURRENT(2, 3);
    _book->setWide(2);
    CURRENT(2, -1);
    _book->next();
    CURRENT(3, 4);
    _book->setWide(4);
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
    _book->setWide(2);
    CURRENT(2, -1);
    _book->next();
    CURRENT(3, 4);
    _book->next();
    CURRENT(5, 6);
    _book->next();
    CURRENT(7, 8);
    _book->next();
    CURRENT(9, 10);

    // Test paging back, and original pairings hold (??? why wouldn't they
    // hold going forward???)
    /// @todo Test this with playing cards
    _book->previous();
    CURRENT(7, 8);
    _book->setWide(8);
    CURRENT(8, -1);
    PAIRED(8, -1);
    PAIRED(9, 10);
    _book->previous();
    CURRENT(7, -1);
    _book->previous();
    CURRENT(5, 6);
    _book->setWide(5);
    CURRENT(6, -1);
    PAIRED(6, -1);
    PAIRED(7, -1);
    _book->previous();
    CURRENT(5, -1);
    _book->previous();
    CURRENT(3, 4);
    _book->previous();
    CURRENT(2, -1);
    _book->previous();
    CURRENT(0, 1);
    _book->setWide(0);
    CURRENT(1, -1);
    _book->previous();
    CURRENT(0, -1);

    delete _book;
    _book = NULL;
}

/**
 * Test that shifting forward works.
 */

/**
 * Test that past shifts are remembered.
 */

/**
 * Test predictive pairing with turning and wide pages.
 */

#include "booktest.moc"
