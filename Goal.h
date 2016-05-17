#ifndef GOAL_H_INCLUDED
#define GOAL_H_INCLUDED

#include "Polynom.h"
#include <string>

class Goal : public Polynom {
public:
    Goal() = default;

    bool parse_and_set(std::string const& str);
    
    std::string const& right() const;
    void right(std::string const& newRight);
    
    bool operator ==(Goal const& o) const;
    friend std::ostream& operator<<(std::ostream& os, Goal const& g);

private:
    std::string _right;
};

#endif
