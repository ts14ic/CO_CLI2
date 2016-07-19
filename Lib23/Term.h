#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "Fraction.h"

class Term {
public:
    Term() = default;
    Term(int idx, bool big = false) : _idx{idx}, _big{big} {}
    Term(int idx, Fraction coeff, bool big = false) : _idx{idx}, _coeff{coeff}, _big{big} {}
    
    int  idx() const { return _idx; }
    bool big() const { return _big; }
    
    Fraction& coeff() { return _coeff; }
    Fraction const& coeff() const { return _coeff; }
    
    bool operator==(Term const& o) const;
    bool operator!=(Term const& o) const;

    friend std::ostream& operator<<(std::ostream& os, Term const& t);

protected:
    int      _idx   = 0;
    Fraction _coeff = {};
    bool     _big   = false;
};

#endif
