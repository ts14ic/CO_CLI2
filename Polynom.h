#ifndef POLYNOM_H_INCLUDED
#define POLYNOM_H_INCLUDED

#include "Term.h"
#include <vector>

class Goal;
class Restriction;

class Polynom {
public:
    Polynom() = default;

    bool parse_and_set(std::string const& str, std::string* rest = nullptr);
    
    void add_term(int idx, Fraction f = {}, bool big = false);
    void add_term(Term const& term);
    void remove_term(int idx);
    void clear_terms();
    
    // accessor only for whole terms
    Term const& term(int idx) const;
    std::vector<Term> const& terms() const;
    
    // both setter and getter for coeffs
    Fraction& coeff(int idx);
    Fraction const& coeff(int idx) const;
    
    // getter only for whether a term is M big
    bool big(int idx) const;
    
    // getters for polynom properties
    std::vector<int> indices() const;
    int last_idx() const;
    int next_idx() const;
    int size() const;
    
    bool operator ==(Polynom const& o) const;

    friend std::ostream& operator<<(std::ostream& os, Polynom const& pnom);

protected:
    std::vector<Term> _terms;

private:
    void simplify();
    void fill_gaps();
};

#endif
