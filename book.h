#ifndef BOOK_H
#define BOOK_H

#include <QObject>

#include <vector>

using std::vector;

/**
 * @brief A model of the physical book: what page it's open to, which pages are
 * facing each other.
 *
 * After being initialized, its pages can be turned, and dual pages
 * need to be identified. Because it is just a digital model, the correct
 * pages may not be facing each other: to compensate, the pages may be shifted
 * forwards, going to the next page and reversing the page parity.
 *
 * Dual pages are two-page spreads in comic books that have been digitally
 * merged together to form one image file. Because the dual pages are not known
 * beforehand, and because the comic book may not follow the constraints of
 * a physical comic book by matching pages correctly, rules need to be used
 * to govern the behaviour of dual pages.
 *
 * In general, dual pages are shown to the user by themselves, thus if the book
 * is currently opened to a dual page, there is only one current page. When
 * paging forward, if a current page is discovered to be a dual page, the first
 * page will always be shown by itself. Likewise, when paging backward and a
 * current page is found to be dual, the later page will be shown by itself.
 * After learning a dual page, the following two pages will become paired
 * together with one exception: the currently displayed pages must not change.
 * This maintains the rule that full pairs occur at the start of the book, and
 * after each dual page, except when the page parity has been explicitly
 * changed.
 *
 * One final exception is that no pages become stranded. Two solitary non-dual
 * pages must not occur in direct sequence: this will make it impossible
 * for them to be viewed together due to the nature of the shifting operation.
 * When dual page is found, this condition must always be maintained.
 *
 * @todo Allow for un-setting pages as dual, in case a prediction is wrong
 */
class Book : public QObject
{
    Q_OBJECT

public:
    Book(QObject *parent = NULL);
    ~Book();

    void reset(int numPages);

    void next();
    void previous();
    void shiftNext();
    void setPage(int page);

    void setDual(int page);
    bool isDual(int page);

    int page0();
    int page1();

    int pairedPage(int page);
    int pairedPageOffset(int page);

    int numPages();

    bool isNextEnabled();
    bool isPreviousEnabled();

signals:
    void changed();
    void dualCausedPageChange();

private:
    enum Pair {Previous = -1, None, Next};
    struct Info
    {
        bool dual;
        Pair pair;
    };

private:
    int _numPages;
    vector<Info> _info;
    int _page0;
    int _page1;
    bool _importantPageIsFirst;
};

#endif
