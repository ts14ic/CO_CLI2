#include "Goal.h"
#include <sstream>

bool Goal::parse_and_set(std::string const& str) {
    Polynom newPolynom;
    std::string leftToParse;
    if(!newPolynom.parse_and_set(str, &leftToParse)) {
        return false;
    }
    
    enum class State {
        start, rel, fin, error
    } state = State::start;
    
    std::string newRight;
    
    std::istringstream is{leftToParse};
    char ch;
    while(is >> ch) {
        switch(state) {
            case State::start:
            if(ch == '=' && is >> ch && ch == '>') {
                state = State::rel;
                continue;
            }
            state = State::error;
            break;
            
            case State::rel: {
                is.putback(ch);
                std::getline(is, newRight);
                if(newRight == "max" || newRight == "min") {
                    state = State::fin;
                }
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
        _right = newRight;
    }
    return success;
}

std::string const& Goal::right() const {
    return _right;
}

void Goal::right(std::string const& newRight) {
    if(newRight == "max" || newRight == "min") {
        _right = newRight;
    }
}

std::ostream& operator<<(std::ostream& os, Goal const& g) {
    if(!std::ostream::sentry{os}) return os;
    if(g.size() < 1) return os << "[Goal:]";

    os << "[Goal:";
    for(int i : g.indices()) {
        os << ' ' << Term(i, g.coeff(i), g.big(i));
    }
    os << " => " << g._right << "]";
    
    return os;
}
