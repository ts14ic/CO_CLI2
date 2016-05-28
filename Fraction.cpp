#include "Fraction.h"
#include <stdexcept>
#include <iostream>
#include <boost/math/common_factor_rt.hpp>

Fraction::Fraction(int n, int d) : _num(n), _den(d) {
    if(d == 0) throw std::domain_error("zero denominator");
    simplify();
}

Fraction::Fraction(int n) : _num(n) {
}

Fraction::Fraction() {
}

int Fraction::num() const { return _num; }
int Fraction::den() const { return _den; }

Fraction Fraction::operator-() const {
    return Fraction(-_num, _den);
}

Fraction& Fraction::operator =(int v) {
    _num = v;
    _den = 1;
    return *this;
}

Fraction Fraction::operator +(Fraction o) const {
    Fraction res = *this;
    int commonDen = res._den * o._den;
    res._num *= commonDen / res._den;
    o._num *= commonDen / o._den;
    res._num += o._num;
    res._den = commonDen;
    res.simplify();
    return res;
}

Fraction Fraction::operator -(Fraction o) const {
    Fraction res = *this;
    int commonDen = res._den * o._den;
    res._num *= commonDen / res._den;
    o._num *= commonDen / o._den;
    res._num -= o._num;
    res._den = commonDen;
    res.simplify();
    return res;
}

Fraction Fraction::operator *(Fraction o) const {
    Fraction res = *this;
    res._num *= o._num;
    res._den *= o._den;
    res.simplify();
    return res;
}

Fraction Fraction::operator /(Fraction o) const {
    Fraction res = *this;
    res._num *= o._den;
    res._den *= o._num;
    res.simplify();
    return res;
}

Fraction& Fraction::operator +=(Fraction o) {
    return *this = *this + o;
}

Fraction& Fraction::operator -=(Fraction o) {
    return *this = *this - o;
}

Fraction& Fraction::operator *=(Fraction o) {
    return *this = *this * o;
}

Fraction& Fraction::operator /=(Fraction o) {
    return *this = *this / o;
}

Fraction Fraction::operator *(int v) const {
    Fraction res = *this;
    res._num *= v;
    res.simplify();
    return res;
}

Fraction Fraction::operator +(int v) const {
    Fraction res = *this;
    res._num += v * res._den;
    res.simplify();
    return res;
}

Fraction Fraction::operator -(int v) const {
    Fraction res = *this;
    res._num -= v * res._den;
    res.simplify();
    return res;
}

Fraction Fraction::operator /(int v) const {
    if(v == 0) throw std::domain_error("Division by zero");
    Fraction res = *this;
    res._den *= v;
    res.simplify();
    return res;
}

Fraction& Fraction::operator +=(int v) {
    return *this = *this + Fraction(v);
}

Fraction& Fraction::operator -=(int v) {
    return *this = *this - Fraction(v);
}

Fraction& Fraction::operator *=(int v) {
    return *this = *this * Fraction(v);
}

Fraction& Fraction::operator /=(int v) {
    return *this = *this / Fraction(v);
}

bool Fraction::operator == (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum == o._num;
}

bool Fraction::operator != (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum != o._num;
}

bool Fraction::operator > (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum > o._num;
}

bool Fraction::operator < (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum < o._num;
}

bool Fraction::operator <= (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum <= o._num;
}

bool Fraction::operator >= (Fraction o) const {
    int tnum = _num * o._den;
    o._num *= _den;
    return tnum >= o._num;
}

bool Fraction::operator == (int o) const {
    return *this == Fraction(o);
}

bool Fraction::operator != (int o) const {
    return *this != Fraction(o);
}

bool Fraction::operator > (int o) const {
    return *this > Fraction(o);
}

bool Fraction::operator < (int o) const {
    return *this < Fraction(o);
}

bool Fraction::operator <= (int o) const {
    return *this <= Fraction(o);
}

bool Fraction::operator >= (int o) const {
    return *this >= Fraction(o);
}

void Fraction::simplify() {
    // self-explanatory
    if(_num == 0) {
        _den = 1;
        return;
    }
    
    // swap the sign to be in numerator
    if(_den < 0) {
        _num = -_num;
        _den = -_den;
    }
    
    int gcd = boost::math::gcd(_num, _den);
    _num /= gcd;
    _den /= gcd;
}

Fraction operator * (int v, Fraction const& o) { return o * v; }
Fraction operator / (int v, Fraction const& o) { return o / v; }
Fraction operator + (int v, Fraction const& o) { return o + v; } 
Fraction operator - (int v, Fraction const& o) { return o - v; }
bool operator == (int v, Fraction const& o) { return o == v; }
bool operator != (int v, Fraction const& o) { return o != v; }
bool operator >  (int v, Fraction const& o) { return o > v; }
bool operator <  (int v, Fraction const& o) { return o < v; }
bool operator >= (int v, Fraction const& o) { return o >= v; }
bool operator <= (int v, Fraction const& o) { return o <= v; }

std::ostream& operator <<(std::ostream& os, Fraction const& frac) {
    if(frac._den == 1) return os << frac._num;
    return os << frac._num << '/' << frac._den;
}

std::istream& operator >>(std::istream& is, Fraction& frac) {
    std::istream::sentry streamOk (is);
    if(!streamOk) return is;
    
    enum class State {
        begin, nominator, slash, denominator, end, error
    };
    
    State state = State::begin;
    int num;
    int den;

    char ch;
    while(is >> ch) {
        switch(state) {
            case State::begin:
            if(std::isdigit(ch)) {
                is.putback(ch);
                is >> num;
                
                if(is.eof()) {
                    frac = num;
                    return is;
                }
                
                state = State::nominator;
                continue;
            }
            else {
                state = State::error;
            }
            break;
            
            
            case State::nominator:
            if(ch == '/') {
                state = State::slash;
                continue;
            }
            else {
                is.putback(ch);
                frac = num;
                return is;
            }
            break;
            
            case State::slash:
            // if next is an integer, and not a 0, return the entered fraction
            // if 0 is entered, it's an error
            // if something different was entered, putback the symbol and a slash, then finish
            if(std::isdigit(ch)) {
                is.putback(ch);
                is >> den;
                
                if(den == 0) {
                    state = State::error;
                    continue;
                }
                else {
                    frac = Fraction(num, den);
                    return is;
                }
            }
            else {
                is.putback(ch);
                is.putback('/');
                return is;
            }
            break;
            
            default:;
        }
    }
    
    return is;
}
