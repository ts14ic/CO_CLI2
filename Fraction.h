#ifndef FRACTION_H_INCLUDED
#define FRACTION_H_INCLUDED

#include <iosfwd>

class Fraction {
public:
    Fraction(int n, int d);
    Fraction(int n);
    Fraction() = default;

    int num() const;
    int den() const;
    
    float as_float() const;

    Fraction operator - () const;
    Fraction& operator = (int v);

    Fraction operator + (Fraction o) const;
    Fraction operator * (Fraction o) const;
    Fraction operator / (Fraction o) const;
    Fraction operator - (Fraction o) const;
    
    bool operator == (Fraction o) const;
    bool operator != (Fraction o) const;
    bool operator >  (Fraction o) const;
    bool operator <  (Fraction o) const;
    bool operator >= (Fraction o) const;
    bool operator <= (Fraction o) const;

    Fraction& operator += (Fraction o);
    Fraction& operator -= (Fraction o);
    Fraction& operator *= (Fraction o);
    Fraction& operator /= (Fraction o);
    
    Fraction operator * (int v) const;
    Fraction operator / (int v) const;
    Fraction operator + (int v) const;
    Fraction operator - (int v) const;
    
    bool operator == (int o) const;
    bool operator != (int o) const;
    bool operator >  (int o) const;
    bool operator <  (int o) const;
    bool operator >= (int o) const;
    bool operator <= (int o) const;
    
    Fraction& operator *= (int v);
    Fraction& operator /= (int v);
    Fraction& operator += (int v);
    Fraction& operator -= (int v);
    
    friend Fraction operator * (int v, Fraction const& o);
    friend Fraction operator / (int v, Fraction const& o);
    friend Fraction operator + (int v, Fraction const& o);
    friend Fraction operator - (int v, Fraction const& o);
    
    friend bool operator == (int v, Fraction const& o);
    friend bool operator != (int v, Fraction const& o);
    friend bool operator >  (int v, Fraction const& o);
    friend bool operator <  (int v, Fraction const& o);
    friend bool operator >= (int v, Fraction const& o);
    friend bool operator <= (int v, Fraction const& o);

    friend std::ostream& operator <<(std::ostream& os, Fraction const& frac);
    friend std::istream& operator >>(std::istream& is, Fraction& frac);
private:
    void normalize();
    void simplify();

    int _num = 0;
    int _den = 1;
};



#endif // FRACTION_H_INCLUDED
