#include "Polynom.h"

#include <sstream>
#include <iostream>

static bool is_sign(char ch) { return ch == '+' || ch == '-'; }
static bool    is_x(char ch) { return ch == 'x' || ch == 'X'; }
static bool  is_rel(char ch) { return ch == '<' || ch == '=' || ch == '>'; }

class TermParseHelper {
public:
    TermParseHelper() = default;

    // setters
    void set_sign(char sign) {
        _signSet = true;
        _sign = sign;
    }
    
    void set_coeff(Fraction coeff) {
        _coeffSet = true;
        _coeff = coeff;
    }
    
    void reset() {
        _signSet = false;
        _coeffSet = false;
    }

    // getters    
    char sign() const { return _sign; }
    bool sign_set() const { return _signSet; }
    Fraction  coeff() const { return _coeff; }
    bool coeff_set() const { return _coeffSet; }

private:
    char _sign;
    bool _signSet = false;
    Fraction  _coeff;
    bool _coeffSet = false;
};

bool Polynom::parse_and_set(std::string const& str, std::string* rest) {
    enum class State {
        start, sign, coeff, x, idx, error
    };
    
    TermParseHelper term;
    std::vector<Term> newTerms;
    
    State state = State::start;
    std::istringstream is {str};
    char ch;
    while(is >> ch) {
        switch(state) {
            case State::start:
            if(is_sign(ch)) {
                term.set_sign(ch);
                state = State::sign;
                continue;
            }
            if(std::isdigit(ch)) {
                is.putback(ch);
                Fraction coeff;
                is >> coeff;
                term.set_coeff(coeff);
                state = State::coeff;
                continue;
            }
            if(is_x(ch)) {
                state = State::x;
                continue;
            }
            state = State::error;
            break;
            
            case State::sign:
            if(std::isdigit(ch)) {
                is.putback(ch);
                Fraction coeff;
                is >> coeff;
                term.set_coeff(coeff);
                state = State::coeff;
                continue;
            }
            if(is_x(ch)) {
                state = State::x;
                continue;
            }
            state = State::error;
            break;
            
            case State::coeff:
            if(is_x(ch)) {
                state = State::x;
                continue;
            }
            state = State::error;
            break;
            
            case State::x:
            if(std::isdigit(ch)) {
                if(term.sign_set() && term.sign() == '-') {
                    if(term.coeff_set()) {
                        term.set_coeff(-term.coeff());
                    }
                    else {
                        term.set_coeff(-1);
                    }
                }
                else {
                    if(!term.coeff_set()) term.set_coeff(1);
                }
                
                is.putback(ch);
                int idx;
                is >> idx;
                state = State::idx;
                
                newTerms.push_back(Term{idx, Fraction(term.coeff())});
                continue;
            }
            state = State::error;
            break;
            
            case State::idx:
            if(is_sign(ch)) {
                term.reset();
                term.set_sign(ch);
                state = State::sign;
                continue;
            }
            if(is_rel(ch) && rest != nullptr) {
                is.putback(ch);
                std::getline(is, *rest);
                continue;
            }
            state = State::error;
            break;
            
            default:;
        }
    }
    
    bool success = (state == State::idx);
    if(success) {
        _terms.swap(newTerms);
        simplify();
        fill_gaps();
    }
    return success;
}

void Polynom::add_term(int idx, Fraction f, bool big) {
    _terms.push_back(Term(idx, f, big));
    simplify();
    fill_gaps();
}

void Polynom::add_term(Term const& t) {
    _terms.push_back(t);
    simplify();
    fill_gaps();
}

void Polynom::remove_term(int idx) {
    for(int i = idx - 1; i >= 0; --i) {
        if(_terms[i].idx() == idx) {
            _terms.erase(_terms.begin() + i);
            break;
        }
    }
}

Term const& Polynom::term(int idx) const {
    for(int i = idx - 1; i >= 0; --i) {
        if(_terms[i].idx() == idx) {
            return _terms[i];
        }
    }
    throw std::range_error{"No term with the index"};
}

std::vector<Term> const& Polynom::terms() const {
    return _terms;
}

Fraction& Polynom::coeff(int idx) {
    for(int i = idx - 1; i >= 0; --i) {
        if(_terms[i].idx() == idx) {
            return _terms[i].coeff();
        }
    }
    throw std::range_error{"No term with the index"};
}

Fraction const& Polynom::coeff(int idx) const{
    for(int i = idx - 1; i >= 0; --i) {
        if(_terms[i].idx() == idx) {
            return _terms[i].coeff();
        }
    }
    throw std::range_error{"No term with the index"};
}

std::vector<int> Polynom::indices() const {
    std::vector<int> ret;
    ret.reserve(_terms.size());
    
    for(auto const& t : _terms) {
        ret.push_back(t.idx());
    }
    
    return ret;
}

int Polynom::last_idx() const {
    if(_terms.empty()) return 0;
    return _terms.back().idx();
}

int Polynom::next_idx() const {
    return last_idx() + 1;
}

bool Polynom::big(int idx) const {
    for(auto f : _terms) {
        if(f.idx() == idx && f.big()) return true;
    }
    return false;
}

int Polynom::size() const { 
    return static_cast<int>(_terms.size());
}

bool Polynom::operator ==(Polynom const& o) const {
    return _terms == o._terms;
}

void Polynom::simplify() {
    // sum simillar
    for(auto i = 0u; i < _terms.size(); ++i) {
        for(auto j = i + 1; j < _terms.size();) {
            if(_terms[i].idx() == _terms[j].idx()) {
                _terms[i].coeff() += _terms[j].coeff();
                _terms.erase(_terms.begin() + j);
            }
            else ++j;
        }
    }
    
    // sort
    for(auto i = 1u; i < _terms.size(); ++i) {
        auto key = _terms[i];
        int j = -1 + i;
        while(j >= 0 && _terms[j].idx() > key.idx()) {
            _terms[j + 1] = _terms[j];
            --j;
        }
        _terms[j + 1] = key;
    }
}

void Polynom::fill_gaps() {
    for(auto i = 0; i < _terms.back().idx(); ++i) {
        if(_terms[i].idx() != i + 1) {
            add_term(i + 1);
        }
    }
}

void Polynom::clear_terms() {
    _terms.clear();
}

std::ostream& operator<<(std::ostream& os, Polynom const& pnom) {
    if(!std::ostream::sentry{os}) return os;
    os << "[Polynom:";
    for(auto const& t : pnom._terms) {
        os << ' ' << t;
    }
    return os << "]";
}
