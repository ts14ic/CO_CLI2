#ifndef RESTRICTION_H_INCLUDED
#define RESTRICTION_H_INCLUDED

#include "Polynom.h"
#include <string>

class Restriction : public Polynom{
public:
    Restriction() = default;
    
    bool parse_and_set(std::string const& str);

    std::string const& rel() const;
    void rel(std::string const& newRel);
    
    Fraction& right();
    Fraction const& right() const;
    
    friend std::ostream& operator<<(std::ostream& os, Restriction const& r);

private:
    std::string _rel;
    Fraction    _right;
};

#endif
