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

int Fraction::num() const { return _num; }
int Fraction::den() const { return _den; }

float Fraction::as_float() const {
    return static_cast<float>(_num) / _den;
}

Fraction::operator float() const {
    return as_float();
}

// Unary operators
Fraction Fraction::operator-() const {
    return Fraction(-_num, _den);
}

// w Fractions operators
Fraction Fraction::operator +(Fraction o) const {
    Fraction res = *this;
    int lcm = boost::math::lcm(res._den, o._den);
    res._num *= lcm / res._den;
    o._num *= lcm / o._den;
    res._num += o._num;
    res._den = lcm;
    res.simplify();
    return res;
}

Fraction Fraction::operator *(Fraction o) const {
    Fraction res = *this;
    int gcd1 = boost::math::gcd(res._num, o._den);
    int gcd2 = boost::math::gcd(res._den, o._num);
    res._num /= gcd1;
    res._den /= gcd2;
    o._den /= gcd1;
    o._num /= gcd2;
    res._num *= o._num;
    res._den *= o._den;
    res.simplify();
    return res;
}

Fraction Fraction::operator -(Fraction o) const {
    return *this + -o;
}

Fraction Fraction::operator /(Fraction o) const {
    return *this * Fraction(o._den, o._num);
}

bool Fraction::operator ==(Fraction o) const {
    return _num == o._num && _den == o._den;
}

bool Fraction::operator <(Fraction o) const {
    int lcm = boost::math::lcm(_den, o._den);
    int tnum = _num * (lcm / _den);
    int onum = o._num * (lcm / o._den);
    return tnum < onum;
}

bool Fraction::operator !=(Fraction o) const {
    return !(*this == o);
}

bool Fraction::operator >(Fraction o) const {
    return *this != o && !(*this < o);
}

bool Fraction::operator <=(Fraction o) const {
    return *this == o || *this < o;
}

bool Fraction::operator >=(Fraction o) const {
    return *this == o || *this > o;
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

// w ints operators
Fraction& Fraction::operator =(int v) {
    _num = v;
    _den = 1;
    return *this;
}

Fraction Fraction::operator +(int v) const {
    Fraction res = *this;
    res._num += v * res._den;
    res.simplify();
    return res;
}

Fraction Fraction::operator *(int v) const {
    Fraction res = *this;
    res._num *= v;
    res.simplify();
    return res;
}

Fraction Fraction::operator -(int v) const {
    return *this + -v;
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

bool Fraction::operator ==(int o) const {
    return *this == Fraction(o);
}

bool Fraction::operator <(int o) const {
    return *this < Fraction(o);
}

bool Fraction::operator !=(int o) const {
    return *this != Fraction(o);
}

bool Fraction::operator >(int o) const {
    return *this > Fraction(o);
}

bool Fraction::operator <=(int o) const {
    return *this <= Fraction(o);
}

bool Fraction::operator >=(int o) const {
    return *this >= Fraction(o);
}

// mirrored w ints operators
Fraction operator *(int v, Fraction const& o) { return o * v; }
Fraction operator /(int v, Fraction const& o) { return o / v; }
Fraction operator +(int v, Fraction const& o) { return o + v; } 
Fraction operator -(int v, Fraction const& o) { return o - v; }
bool operator ==(int v, Fraction const& o) { return o == v; }
bool operator !=(int v, Fraction const& o) { return o != v; }
bool operator > (int v, Fraction const& o) { return o > v; }
bool operator < (int v, Fraction const& o) { return o < v; }
bool operator >=(int v, Fraction const& o) { return o >= v; }
bool operator <=(int v, Fraction const& o) { return o <= v; }

void Fraction::normalize() {
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
}

void Fraction::simplify() {
    normalize();
    if(_num == 0) return;
    
    int gcd = boost::math::gcd(_num, _den);
    _num /= gcd;
    _den /= gcd;
}

std::ostream& operator <<(std::ostream& os, Fraction const& frac) {
    std::ostream::sentry sentry {os};
    if(!sentry) return os;
    
    if(frac._den == 1) return os << frac._num;
    return os << frac._num << '/' << frac._den;
}

inline namespace helpers {
    bool is_sign(char ch) {
        return ch == '-' || ch == '+';
    }
}

std::istream& operator >>(std::istream& is, Fraction& frac) {
    std::istream::sentry streamOk (is);
    if(!streamOk) return is;
    
    enum class State {
        begin, sign, nominator, slash, error
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
                else {
                    state = State::nominator;
                    continue;
                }
            }
            else if(is_sign(ch)) {
                state = State::sign;
                continue;
            }
            else {
                state = State::error;
            }
            break;
            
            
            case State::sign:
            if(std::isdigit(ch)) {
                is.putback(ch);
                is.unget();
                is >> num;
                
                if(is.eof()) {
                    frac = num;
                    return is;
                }
                else {
                    state = State::nominator;
                    continue;
                }
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
                // put the slash back
                is.unget();
                return is;
            }
            break;
            
            default:;
        }
    }
    
    // we can't be here after a successfull read
    is.setstate(is.failbit);
    return is;
}
