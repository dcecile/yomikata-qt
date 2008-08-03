#ifndef BOOK_H
#define BOOK_H

#include <vector>

using std::vector;

/**
 *
 *
 * Wide page rules:
 * - after setting wide, current parity will not change
 *     - special case: going back, and sooner page becomes wide -- no parity
 *       change (opposed to moving forward)
 * - pairs always follow book start, and wide pages
 */
class Book
{
public:
    Book(int numPages);
    ~Book();

    void next();
    void previous();

    void setWide(int page);

    int page0() const;
    int page1() const;

    int pairedPage(int page) const;

private:
    enum Pair {Previous = -1, None, Next};
    struct Info
    {
        bool wide;
        Pair pair;
    };

private:
    int _numPages;
    vector<Info> _info;
    int _page0;
    int _page1;
    bool _goingForward;
};

#endif
