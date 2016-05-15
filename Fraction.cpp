#include "Fraction.h"
#include <stdexcept>
#include <iostream>

Fraction::Fraction(int n, int d) : _num(n), _den(d) {
    if(d == 0) throw std::logic_error("zero denominator");
    simplify();
}

Fraction::Fraction(int n) : _num(n) {
    simplify();
}

Fraction::Fraction() {
    simplify();
}

int Fraction::num() const { return _num; }
int Fraction::den() const { return _den; }

Fraction Fraction::operator-() const {
    return Fraction(-_num, _den);
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
	Fraction res = *this;
	if(v == 0) throw std::logic_error("Division by zero");
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
    if(_num == 0) {
        _den = 1;
        return;
	}
    if(_den < 0) {
        _num = -_num;
        _den = -_den;
	}
	int cap = _num * _den;
	if(cap < 0) cap = -cap;
	for(int i = cap; i > 1; --i) {
		if((_den % i == 0) && (_num % i == 0)) {
			_den /= i;
			_num /= i;
		}
	}
}

std::ostream& operator <<(std::ostream& os, Fraction const& frac) {
	if(frac._den == 1) return os << frac._num;
	return os << frac._num << '/' << frac._den;
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
