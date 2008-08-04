#include "revision.h"

Revision::Revision()
{
    _epoch = 0;
}


Revision::~Revision()
{
}

void Revision::advance()
{
    _epoch++;
}

Revision::Token Revision::token() const
{
    return Token(_epoch);
}

Revision::Token::Token(int baseEpoch)
    : _baseEpoch(baseEpoch)
{
}

bool Revision::Token::isValid()
{
    return false;
}
