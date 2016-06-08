#ifndef FRACTION_H_INCLUDED
#define FRACTION_H_INCLUDED

#include <iosfwd>

class Fraction {
public:
    using int_t = long;

    Fraction(int_t n, int_t d);
    Fraction(int_t n);
    Fraction() = default;

    int_t num() const;
    int_t den() const;
    
    float as_float() const;
    explicit operator float() const;

    // Unary operators
    Fraction operator -() const;

    // w Fractions operators
    Fraction operator +(Fraction o) const;
    Fraction operator *(Fraction o) const;
    Fraction operator -(Fraction o) const;
    Fraction operator /(Fraction o) const;
    
    bool operator ==(Fraction o) const;
    bool operator < (Fraction o) const;
    bool operator !=(Fraction o) const;
    bool operator > (Fraction o) const;
    bool operator >=(Fraction o) const;
    bool operator <=(Fraction o) const;
    
    Fraction& operator +=(Fraction o);
    Fraction& operator -=(Fraction o);
    Fraction& operator *=(Fraction o);
    Fraction& operator /=(Fraction o);
    
    // w ints operators
    Fraction& operator =(int_t v);
    
    Fraction operator *(int_t v) const;
    Fraction operator /(int_t v) const;
    Fraction operator +(int_t v) const;
    Fraction operator -(int_t v) const;
    
    Fraction& operator *=(int_t v);
    Fraction& operator /=(int_t v);
    Fraction& operator +=(int_t v);
    Fraction& operator -=(int_t v);
    
    bool operator ==(int_t o) const;
    bool operator < (int_t o) const;
    bool operator !=(int_t o) const;
    bool operator > (int_t o) const;
    bool operator >=(int_t o) const;
    bool operator <=(int_t o) const;
    
    //  mirrored w ints operators
    friend Fraction operator *(int_t v, Fraction const& o);
    friend Fraction operator /(int_t v, Fraction const& o);
    friend Fraction operator +(int_t v, Fraction const& o);
    friend Fraction operator -(int_t v, Fraction const& o);
    
    friend bool operator ==(int_t v, Fraction const& o);
    friend bool operator < (int_t v, Fraction const& o);
    friend bool operator !=(int_t v, Fraction const& o);
    friend bool operator > (int_t v, Fraction const& o);
    friend bool operator >=(int_t v, Fraction const& o);
    friend bool operator <=(int_t v, Fraction const& o);
    
    friend std::ostream& operator <<(std::ostream& os, Fraction const& frac);
    friend std::istream& operator >>(std::istream& is, Fraction& frac);
private:
    void normalize();
    void simplify();

    int_t _num = 0;
    int_t _den = 1;
};

#endif // FRACTION_H_INCLUDED
