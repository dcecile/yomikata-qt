#include "booktest.h"

#include <QTest>

#include "book.h"

#define PAIRED(page, pair) \
    do {QCOMPARE(_book->pairedPage((page)), (pair));} while (false)

#define CURRENT(a, b) \
    do {QCOMPARE(QCOMPARE(_book->page0(), (a)); \
    QCOMPARE(_book->page1(), (b));} while (false)

BookTest::BookTest()
{
}

BookTest::~BookTest()
{
}

void BookTest::paired(int page, int pair)
{
    QCOMPARE(_book->pairedPage(page), pair);
}

void BookTest::current(int page0, int page1)
{
    QCOMPARE(_book->page0(), page0);
    QCOMPARE(_book->page1(), page1);
}

/**
 * Test simple pairing.
 */
void BookTest::simple()
{
    // Check for odd pages book
    _book = new Book(5);

    // Each page is paired except the last
    PAIRED(0, 2);
    paired(0, 1);
    paired(1, 2);
    paired(2, 3);
    paired(3, 2);
    paired(4, -1);

    delete _book;

    // Check for even pages book
    _book = new Book(6);

    // Each page is paired
    paired(0, 1);
    paired(1, 0);
    paired(2, 3);
    paired(3, 2);
    paired(4, 5);
    paired(5, 4);

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
    paired(0, 1);
    paired(1, 0);
    paired(2, -1);
    paired(3, 4);
    paired(4, 3);
    paired(5, 6);
    paired(6, 5);

    // Set two more paired, some pairs can't form
    _book->setWide(1);
    _book->setWide(6);
    paired(0, -1);
    paired(1, -1);
    paired(2, -1);
    paired(3, 4);
    paired(4, 3);
    paired(5, -1);
    paired(6, -1);

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
    current(0, 1);
    _book->next();
    current(2, 3);
    _book->next();
    current(4, -1);

    // And back
    _book->previous();
    current(2, 3);
    _book->previous();
    current(0, 1);

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
    current(0, 1);

    // Set a page wide
    _book->setWide(1);

    // Page to the end
    current(0, -1);
    _book->next();
    current(1, -1);
    _book->next();
    current(2, 3);
    _book->setWide(2);
    current(2, -1);
    _book->next();
    current(3, 4);

    // And back
    _book->previous();
    current(2, -1);
    _book->previous();
    current(1, -1);
    _book->previous();
    current(0, -1);

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
    current(0, 1);
    _book->next();
    current(2, 3);

    // Learn about a wide page
    _book->setWide(8);

    // Page forward and hit the wide page
    _book->next();
    current(4, 5);
    _book->next();
    current(6, 7);
    _book->next();
    current(8, -1);
    _book->next();
    current(9, 10);
    _book->next();
    current(11, -1);

    delete _book;

    // Use a big book
    _book = new Book(12);

    // Page forward a bit
    current(0, 1);
    _book->next();
    current(2, 3);

    // Learn about a new wide page
    _book->setWide(7);

    // Page forward and hit the wide page, the preceding pair can't form
    _book->next();
    current(4, 5);
    _book->next();
    current(6, -1);
    _book->next();
    current(7, -1);
    _book->next();
    current(8, 9);
    _book->next();
    current(10, 11);

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
    current(0, 1);
    _book->next();
    current(2, 3);

    // Learn about a wide page from before
    _book->setWide(0);

    // The current parity will not change
    //CURRENT(2, 3);
    paired(0, -1);
    paired(1, -1);
    paired(2, 3);
    paired(4, 5);
    paired(6, 7);
    paired(8, 9);
    paired(10, -1);

    // Page forward
    _book->next();
    current(4, 5);

    // Reset
    _book->setWide(4);
    current(4, -1);

    // Page forward a bit
    _book->next();
    current(5, 6);
    _book->next();
    current(7, 8);

    // Learn about a wide page from before
    _book->setWide(6);

    // The current parity will not change
    current(7, 8);
    paired(0, -1);
    paired(1, -1);
    paired(2, 3);
    paired(4, -1);
    paired(5, -1);
    paired(6, -1);
    paired(7, 8);
    paired(9, 10);

    // Page to the end
    _book->next();
    current(9, 10);

    delete _book;
    _book = NULL;
}

/**
 * Test that a page of the current pair being set wide will keep the correct current page.
 */
void BookTest::currentWide()
{
    _book = new Book(7);

    // Test paging forward
    current(0, 1);
    _book->next();
    current(2, 3);
    _book->setWide(2);
    current(2, -1);
    _book->next();
    current(3, 4);
    _book->setWide(4);
    current(3, -1);
    _book->next();
    current(4, -1);
    _book->next();
    current(5, 6);

    delete _book;

    _book = new Book(11);

    // Page forward
    current(0, 1);
    _book->next();
    current(2, 3);
    _book->setWide(2);
    current(2, -1);
    _book->next();
    current(3, 4);
    _book->next();
    current(5, 6);
    _book->next();
    current(7, 8);
    _book->next();
    current(9, 10);

    // Test paging back, and original pairings hold (??? why wouldn't they hold going forward???)
    /// @todo Test this with playing cards
    _book->previous();
    current(7, 8);
    _book->setWide(8);
    current(8, -1);
    paired(8, -1);
    paired(9, 10);
    _book->previous();
    current(7, -1);
    _book->previous();
    current(5, 6);
    _book->setWide(5);
    current(6, -1);
    paired(6, -1);
    paired(7, -1);
    _book->previous();
    current(5, -1);
    _book->previous();
    current(3, 4);
    _book->previous();
    current(2, -1);
    _book->previous();
    current(0, 1);
    _book->setWide(0);
    current(1, -1);
    _book->previous();
    current(0, -1);

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
