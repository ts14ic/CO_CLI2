#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include "Goal.h"
#include "Restriction.h"
#include <vector>

class Solver {
public:
    Solver() = default;
    
    bool set_goal(std::string const& str);
    bool add_restriction(std::string const& str);
    Solver& invert_to_dual();
    
    struct Step;
    std::vector<Step> solve();
    
    friend std::ostream& operator<<(std::ostream& os, Solver const& solver);

private:
    void append_preferred();
    void append_artificial();
    
    Goal                     _goal;
    std::vector<Term>        _sel;
    std::vector<Restriction> _restrs;
    std::vector<int>         _initialBasis;
};

class Solver::Step {
public:
    Goal                     goal;
    std::vector<Term>        sel;
    std::vector<Restriction> restrs;
    Polynom                  pprice;
    Polynom                  mprice;
    std::vector<Term>        basis;
    Fraction                 w;
    Fraction                 m;
    
    bool valid() const;
    void mark_as_valid();
    
    bool operator ==(Step const& o) const;
    
private:
    bool _valid = false;
};

void print_step(std::ostream& os, Solver::Step const& s, bool price, bool newline = false);

#endif
