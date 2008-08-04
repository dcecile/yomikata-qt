#ifndef REVISION_H
#define REVISION_H

class RevisionToken
{
public:
    ~RevisionToken()
    {
    }

protected:
    RevisionToken(int epoch)
    {
        _epoch = epoch;
    }

    RevisionToken copy() const
    {
        return RevisionToken(_epoch);
    }

    bool matches(const RevisionToken &other) const
    {
        return _epoch == other._epoch;
    }

protected:
    int _epoch;
};

class AdvancingRevisionToken : public RevisionToken
{
public:
    AdvancingRevisionToken()
        : RevisionToken(0)
    {
    }
    ~AdvancingRevisionToken()
    {
    }

    void advance()
    {
        _epoch++;
    }

    RevisionToken token() const
    {
        return copy();
    }

    bool isTokenValid(const RevisionToken &token) const
    {
        return matches(token);
    }
};

class Revision
{
public:
    class Token;

public:
    Revision();
    ~Revision();

    void advance();

    Token token() const;

public:
    class Token
    {
    public:
        bool isValid();

    //private:
        Token(int baseEpoch);

    private:
        const int &_baseEpoch;
        int _epochToken;
    };

private:
    int _epoch;
};

#endif
