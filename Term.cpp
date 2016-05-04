#include "Term.h"
#include <iostream>

bool Term::operator==(Term const& o) const {
    return _idx == o._idx && _coeff == o._coeff && _big == o._big;
}

bool Term::operator!=(Term const& o) const {
    return !operator==(o);
}

std::ostream& operator <<(std::ostream& os, Term const& t) {
    if(!std::ostream::sentry{os}) return os;
    return os << t._coeff << (t._big ? "M" : "" ) << "{X" << t._idx << "}";
}
