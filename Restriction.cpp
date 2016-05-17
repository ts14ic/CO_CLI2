#include "Restriction.h"
#include <sstream>

static bool is_sign(char ch) { return ch == '-' || ch == '+'; }
static bool  is_rel(char ch) { return ch == '=' || ch == '>' || ch == '<'; }

bool Restriction::parse_and_set(std::string const& str) {
    Polynom newPolynom;
    std::string leftToParse;
    if(!newPolynom.parse_and_set(str, &leftToParse)) {
        return false;
    }
    
    enum class State {
        start, halfRel, rel, sign, fin, error
    } state = State::start;
    
    std::string newRel;
    Fraction newRight;
    bool negative = false;

    std::istringstream is {leftToParse};
    char ch;
    while(is >> ch) {
        switch(state) {
            case State::start:
            if(is_rel(ch)) {
                newRel += ch;
                state = State::halfRel;
                continue;
            }
            state = State::error;
            break;
            
            case State::halfRel:
            if(ch == '=') {
                newRel += ch;
                state = State::rel;
                continue;
            }
            state = State::error;
            break;
            
            case State::rel:
            if(std::isdigit(ch)) {
                is.putback(ch);
                int right;
                is >> right;
                newRight = Fraction{right};
                state = State::fin;
                continue;
            }
            if(is_sign(ch)) {
                if(ch == '-') negative = true;
                state = State::sign;
                continue;
            }
            state = State::error;
            break;
            
            case State::sign:
            if(std::isdigit(ch)) {
                is.putback(ch);
                int right;
                is >> right;
                if(negative) right = -right;
                newRight = Fraction{right};
                state = State::fin;
                continue;
            }
            state = State::error;
            break;
            
            default: return false;
        }
    }
    
    bool success = (state == State::fin);
    if(success) {
        _terms = newPolynom.terms();
        _rel   = newRel;
        _right = newRight;
    }
    return success;
}

std::string const& Restriction::rel() const {
    return _rel;
}

void Restriction::rel(std::string const& newRel) {
    if(newRel == ">=" || newRel == "<=" || newRel == "==") {
        _rel = newRel;
    }
}

Fraction& Restriction::right() {
    return _right;
}

Fraction const& Restriction::right() const {
    return _right;
}

bool Restriction::operator ==(Restriction const& o) const {
    return _terms == o._terms && _rel == o._rel && _right == o._right;
}

std::ostream& operator<<(std::ostream& os, Restriction const& r) {
    if(!std::ostream::sentry{os}) return os;
    if(r.size() < 1) return os << "[Restriction:]";
    os << "[Restriction:";
    for(int i : r.indices()) {
        os << ' ' << r.term(i);
    }
    return os << ' ' << r._rel << ' ' << r._right << "]";
}
