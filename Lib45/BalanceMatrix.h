#ifndef BALANCEMATRIX_H_INCLUDED
#define BALANCEMATRIX_H_INCLUDED

#include <initializer_list>
#include <vector>
#include <iosfwd>

class BalanceMatrix {
public:
    BalanceMatrix() = default;
    bool set(std::initializer_list<std::vector<int>> const& rows);
    bool set(std::vector<std::vector<int>> const& rows);
    
    std::vector<std::vector<int>> get_key0_by_nw_method(
        std::vector<int>* outProds   = nullptr,
        std::vector<int>* outConsums = nullptr,
        std::vector<std::vector<int>>* outCosts = nullptr
    ) const;
    
    std::vector<std::vector<int>> get_key0_by_min_method(
        std::vector<int>* outProds   = nullptr,
        std::vector<int>* outConsums = nullptr,
        std::vector<std::vector<int>>* outCosts = nullptr
    ) const;
    
    enum class Meth {
        NW, Min
    };
    
    struct Step;
    std::vector<Step> solve(Meth const& m) const;

    friend std::ostream& operator <<(std::ostream& os, BalanceMatrix const& m);
private:
    std::vector<std::vector<int>> _costs;
    std::vector<int> _prods;
    std::vector<int> _consums;
};

struct BalanceMatrix::Step {
    std::vector<std::vector<int>> X;
    std::vector<std::vector<int>> D;
    int W = 0;
    
    bool valid() const;
};

void print_vec2d(
    std::ostream& os, 
    std::vector<std::vector<int>> const& vec2d, 
    bool eps_is_minus_one = false
);

#endif
