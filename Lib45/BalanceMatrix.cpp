#include "BalanceMatrix.h"

#include <iostream>
#include <iomanip>
#include <initializer_list>
#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/optional.hpp>

using std::pair;
using std::tuple;
using std::vector;
using boost::optional;
using OptInt = optional<int>;

template <typename T>
using Vector2D = vector<vector<T>>;

using Line = vector<int>;
using Matrix = Vector2D<int>;
using Cell = pair<int, int>;
using Link = pair<int, int>;

static constexpr int EPS = -1;

std::ostream& operator <<(std::ostream& os, BalanceMatrix const& m) {
    std::ostream::sentry osOk {os};
    if(!osOk) return os;
    
    os << "[Balance:\n";
    for(auto r = 0u; r < m._costs.size(); ++r) {
        for(int c : m._costs[r]) {
            os << std::setw(4) << c;
        }
        os << "|" << std::setw(4) << m._prods[r] << '\n';
    }
    if(!m._consums.empty()) {
        for(int c : m._consums) {
            os << std::setw(4) << c;
        }
        os << "\n]";
    }
    
    return os;
}

void print_vec2d(std::ostream& os, std::vector<std::vector<int>> const& vec2d, bool eps_is_minus_one) {
    std::ostream::sentry osOk {os};
    if(!osOk) return;
    
    os << "[";
    for(auto const& row : vec2d) {
        os << "\n";
        for(auto const& item : row) {
            if(eps_is_minus_one && item <= EPS) os << std::setw(3) << std::abs(item) << 'E';
            else os << std::setw(4) << item;
        }
    }
    os << "\n]";
}

bool BalanceMatrix::set(std::initializer_list<std::vector<int>> const& rows) {
    if(rows.size() < 2) return false;
    
    vector<vector<int>> rowsCopy;
    for(auto const& row : rows) {
        auto rowCopy = row;
        rowsCopy.push_back(rowCopy);
    }
    
    return set(rowsCopy);
}

bool BalanceMatrix::set(vector<vector<int>> const& rows) {
    if(rows.size() < 2) return false;
    
    Matrix costs;
    Line prods;
    Line consums;
    
    for(auto r = rows.begin(); r != rows.end() - 1; ++r) {
        costs.push_back(*r);
        costs.back().pop_back();
        prods.push_back(r->back());
    }
    consums = *(rows.end() - 1);
    
    auto length = costs.front().size();
    if(prods.size() != costs.size()) return false;
    if(consums.size() != length) return false;
    for(auto i = 1u; i < costs.size(); ++i) {
        if(costs[i].size() != length) return false;
    }
    
    _costs = costs;
    _prods = prods;
    _consums = consums;
    
    return true;
}

static void flatten(Matrix& costs, vector<int>& prods, vector<int>& consums) {
    auto prodsSum = std::accumulate(prods.begin(), prods.end(), 0);
    auto consumsSum = std::accumulate(consums.begin(), consums.end(), 0);
    
    if(consumsSum > prodsSum) {
        prods.push_back(consumsSum - prodsSum);
        
        Line newRow (consums.size());
        costs.push_back(newRow);
    }
    else if(prodsSum > consumsSum) {
        consums.push_back(prodsSum - consumsSum);
        
        for(auto& r : costs) {
            r.push_back(0);
        }
    }
}

Matrix BalanceMatrix::get_key0_by_nw_method(
    Line* outProds,
    Line* outConsums,
    Matrix* outCosts
) const {
    auto costs   = _costs;
    auto prods   = _prods;
    auto consums = _consums;
    
    flatten(costs, prods, consums);
    
    auto prodsNum    = prods.size();
    auto consumsNum  = consums.size();
    
    Matrix ret (prodsNum, Line(consumsNum, 0));
    
    for(auto i = 0u; i < prodsNum; ++i) {
        for(auto j = 0u; j < consumsNum; ++j) {
            auto x = std::min(prods[i], consums[j]);
            prods[i] -= x;
            consums[j] -= x;
            ret[i][j] = x;
        }
    }
    
    if(outProds) {
        *outProds = prods;
    }
    if(outConsums) {
        *outConsums = consums;
    }
    if(outCosts) {
        *outCosts = costs;
    }
    
    return ret;
}

static vector<Cell> enumerate_by_min(Matrix const& costs) {
    vector<Cell> ret;
    
    for(auto i = 0u; i < costs.size(); ++i) {
        for(auto j = 0u; j < costs[0].size(); ++j) {
            ret.emplace_back(i, j);
        }
    }
    
    std::sort(ret.begin(), ret.end(), [&ret, &costs](Cell const& a, Cell const& b){
        if(costs[a.first][a.second] == costs[b.first][b.second]) {
            return a.first + a.second < b.first + b.second;
        }
        else {
            return costs[a.first][a.second] < costs[b.first][b.second];
        }
    });
    
    auto firstNotZero = std::find_if(ret.begin(), ret.end(),
    [&ret, &costs](Cell const& a){
        return costs[a.first][a.second] > 0;
    });
    
    std::rotate(ret.begin(), firstNotZero, ret.end());
    
    return ret;
}

Matrix BalanceMatrix::get_key0_by_min_method(
    Line* outProds,
    Line* outConsums,
    Matrix* outCosts
) const {
    auto costs   = _costs;
    auto prods   = _prods;
    auto consums = _consums;
    
    flatten(costs, prods, consums);
    
    auto prodsNum    = prods.size();
    auto consumsNum  = consums.size();
    
    Matrix ret (prodsNum, vector<int>(consumsNum, 0));
    auto indices = enumerate_by_min(costs);
    
    for(auto const& c : indices) {
        auto x = std::min(prods[c.first], consums[c.second]);
        prods[c.first] -= x;
        consums[c.second] -= x;
        ret[c.first][c.second] = x;
    }
    
    if(outProds) {
        *outProds = prods;
    }
    if(outConsums) {
        *outConsums = consums;
    }
    if(outCosts) {
        *outCosts = costs;
    }
    
    return ret;
}

static vector<Link> 
form_links(Matrix const& x) {
    vector<Link> ret;
    
    for(auto i = 0u; i < x.size(); ++i) { // consums
        for(auto j = 0u; j < x[0].size(); ++j) { // prods
            if(x[i][j] != 0) {
                ret.emplace_back(i, j);
            }
        }
    }
    
    return ret;
}

static bool add_link(vector<Link>& links, Matrix& X, vector<OptInt> const& U, vector<OptInt> const& V) {
    for(auto i = 0u; i < U.size(); ++i) {
        if(!U[i]) {
            for(auto j = 0u; j < V.size(); ++j) {
                if(V[j]) {
                    links.emplace_back(i, j);
                    X[i][j] = EPS;
                    return true;
                }
            }
        }
    }
    return false;
}

static void fill_uv(
    Matrix const& costs,
    Matrix& X,
    vector<int>& outU,
    vector<int>& outV
) {
    vector<OptInt> U (costs.size());
    vector<OptInt> V (costs[0].size());
    
    auto links = form_links(X);
    
    U[0] = 0;
    
    // any change triggers loop repetition
    for(bool needReps = true; needReps;) {
        needReps = false;
        
        for(auto const& l : links) {
            auto i = l.first;
            auto j = l.second;
            
            if(U[i] && !V[j]) {
                V[j] = *U[i] + costs[i][j];
                needReps = true;
            }
            else if(V[j] && !U[i]) {
                U[i] = *V[j] - costs[i][j];
                needReps = true;
            }
        }
        
        if(!needReps) {
            needReps = add_link(links, X, U, V);
        }
    }
    
    outU.clear(); 
    outV.clear();
    
    for(auto i = 0u; i < U.size(); ++i) {
        outU.emplace_back(*U[i]);
    }
    for(auto j = 0u; j < V.size(); ++j) {
        outV.emplace_back(*V[j]);
    }
}

static Matrix
get_price0(Matrix const& costs, vector<int> const& U, vector<int> const& V) {
    Matrix ret (costs.size(), vector<int>(costs[0].size()));
    
    for(auto i = 0u; i < costs.size(); ++i) {
        for(auto j = 0u; j < costs[0].size(); ++j) {
            ret[i][j] = costs[i][j] - V[j] + U[i];
        }
    }
    
    return ret;
}

bool all_positive(Matrix const& D) {
    for(auto const& r : D) {
        for(int i : r) {
            if(i < 0) return false;
        }
    }
    return true;
}

Cell most_negative_element(Matrix const& D) {
    int r = 0;
    int c = 0;
    
    for(auto i = 0u; i < D.size(); ++i) {
        for(auto j = 0u; j < D[0].size(); ++j) {
            if(D[i][j] < D[r][c]) {
                r = i;
                c = j;
            }
        }
    }
    
    return {r, c};
}

bool even(int i) { return i % 2 == 0; }

vector<Cell> mark_forbidden(
    Matrix const& x, Cell const& mn
) {
    vector<Cell> ret;
    
    vector<vector<bool>> notZero (x.size(), vector<bool>(x[0].size(), false));
    for(auto i = 0u; i < x.size(); ++i) {
        for(auto j = 0u; j < x[0].size(); ++j) {
            if(x[i][j] != 0) notZero[i][j] = true;
        }
    }
    notZero[mn.first][mn.second] = true;
    
    auto direction = 0;
    bool needStriking = true;
    
    while(needStriking) {
        if(direction > 0) needStriking = false;
        
        // horizontal
        if(even(direction)) {
            for(auto i = 0u; i < x.size(); ++i) { // for all rows
                int count = 0;
                for(auto j = 0u; j < x[0].size(); ++j) {
                    if(notZero[i][j]) {
                        ++count;
                    }
                }
                
                if(count == 1) {
                    for(auto j = 0u; j < x[0].size(); ++j) {
                        ret.emplace_back(i, j);
                        notZero[i][j] = false;
                    }
                    needStriking = true;
                }
            }
        }
        else {
            for(auto j = 0u; j < x[0].size(); ++j) {
                int count = 0;
                for(auto i = 0u; i < x.size(); ++i) {
                    if(notZero[i][j]) {
                        ++count;
                    }
                }
                
                if(count == 1) {
                    for(auto i = 0u; i < x.size(); ++i) {
                        ret.emplace_back(i, j);
                        notZero[i][j] = false;
                    }
                    needStriking = true;
                }
            }
        }
        
        if(ret.size() == notZero.size() * notZero[0].size()) {
            break;
        }
        
        ++direction;
    }
    
    std::sort(ret.begin(), ret.end());
    auto last = std::unique(ret.begin(), ret.end());
    ret.erase(last, ret.end());
    
    return ret;
}

template <typename T, typename Container>
static bool contains(Container const& container, T const& val) {
    for(auto const& v : container) {
        if(v == val) return true;
    }
    return false;
}

int count_allowed(Matrix const& x, vector<Cell> const& fb, Cell const& s) {
    int num = 0;
    
    unsigned si = s.first;
    unsigned sj = s.second;
    
    for(auto i = 0u; i < x.size(); ++i) {
        for(auto j = 0u; j < x[0].size(); ++j) {
            if(i == si && j == sj) {
                ++num;
            }
            else {
                if(x[i][j] != 0 && !contains(fb, Cell{i, j})) ++num;
            }
        }
    }
    
    return num;
}

enum class Sign {
    none, plus, minus
};

Sign next_sign(Sign& c) {
    if(c == Sign::plus) {
        c = Sign::minus;
    }
    else {
        c = Sign::plus;
    }
    
    return c;
}

void tryDown(
    int& count,
    int& si, int const sj, 
    vector<vector<Sign>>& signs, 
    Sign& currentSign,
    Matrix const& x,
    vector<Cell> const& fb
){
    int lastI = si;
    for(auto i = 1u + si; i < x.size(); ++i) {
        if(x[i][sj] != 0 && !contains(fb, Cell(i, sj))) {
            lastI = i;
        } 
    }
    if(lastI != si && signs[lastI][sj] == Sign::none) {
        ++count;
        si = lastI;
        signs[si][sj] = next_sign(currentSign);
    }
};

void tryUp(
    int& count,
    int& si, int const sj, 
    vector<vector<Sign>>& signs, 
    Sign& currentSign,
    Matrix const& x,
    vector<Cell> const& fb
){
    int lastI = si;
    for(auto i = si - 1; i >= 0; --i) {
        if(x[i][sj] != 0 && !contains(fb, Cell(i, sj))) {
            lastI = i;
        } 
    }
    if(lastI != si && signs[lastI][sj] == Sign::none) {
        ++count;
        si = lastI;
        signs[si][sj] = next_sign(currentSign);
    }
};

void tryRight(
    int& count,
    int const si, int& sj, 
    vector<vector<Sign>>& signs, 
    Sign& currentSign,
    Matrix const& x,
    vector<Cell> const& fb
){
    int lastJ = sj;
    for(auto j = 1u + sj; j < x[0].size(); ++j) {
        if(x[si][j] != 0 && !contains(fb, Cell(si, j))) {
            lastJ = j;
        } 
    }
    if(lastJ != sj && signs[si][lastJ] == Sign::none) {
        ++count;
        sj = lastJ;
        signs[si][sj] = next_sign(currentSign);
    }
};

void tryLeft(
    int& count,
    int const si, int& sj, 
    vector<vector<Sign>>& signs, 
    Sign& currentSign,
    Matrix const& x,
    vector<Cell> const& fb
){
    int lastJ = sj;
    for(auto j = sj - 1; j >= 0; --j) {
        if(x[si][j] != 0 && !contains(fb, Cell(si, j))) {
            lastJ = j;
        } 
    }
    if(lastJ != sj && signs[si][lastJ] == Sign::none) {
        ++count;
        sj = lastJ;
        signs[si][sj] = next_sign(currentSign);
    }
};

static Cell least_minus(
    Matrix const& x,
    vector<vector<Sign>> const& signs,
    Cell const& mn
) {
    Cell ret = mn; // least minus marked
    
    for(auto i = 0u; i < x.size(); ++i) {
        for(auto j = 0u; j < x[0].size(); ++j) {
            if(signs[i][j] == Sign::minus) {
                if(x[i][j] < x[ret.first][ret.second] || 
                   signs[ret.first][ret.second] == Sign::plus)
                {                    
                    ret.first = i;
                    ret.second = j;
                }
            }
        }
    }
    
    return ret;
}

static Matrix advance_x(
    Matrix const& prevX, 
    Cell const& mn
) {    
    auto fb = mark_forbidden(prevX, mn);
    
    Vector2D<Sign> signs (prevX.size(), vector<Sign>(prevX[0].size(), Sign::none));
    int si = mn.first;
    int sj = mn.second;
    signs[si][sj] = Sign::plus;
    
    int count = 1;
    int num = count_allowed(prevX, fb, mn);
    
    auto currentSign = Sign::plus;
    
    while(count < num) {
        tryUp   (count, si, sj, signs, currentSign, prevX, fb);
        tryDown (count, si, sj, signs, currentSign, prevX, fb);
        tryRight(count, si, sj, signs, currentSign, prevX, fb);
        tryLeft (count, si, sj, signs, currentSign, prevX, fb);
    }
    
    Matrix ret = prevX;
    
    auto lm = least_minus(prevX, signs, mn);
    auto delta = prevX[lm.first][lm.second];
    
    bool nullified = false;
    for(auto i = 0u; i < prevX.size(); ++i) {
        for(auto j = 0u; j < prevX[0].size(); ++j) {
            if(signs[i][j] == Sign::minus) {
//                ret[i][j] -= delta + (prevX[i][j] < 0 ? -EPS : 0);
                if(delta > 0) {
                    if(prevX[i][j] >= 0) ret[i][j] -= delta;
                    else ret[i][j] = -delta;
                }
                else if(delta <= EPS) {
                    if(prevX[i][j] <= 0) ret[i][j] -= delta;
                    else ; // ignored
                }
                
                if(ret[i][j] == 0) {
                    if(!nullified) nullified = true;
                    else ret[i][j] = EPS;
                }
            }
            else if(signs[i][j] == Sign::plus) {
//                ret[i][j] += delta + (prevX[i][j] < 0 ? -EPS : 0);
                
                if(delta > 0) {
                    if(prevX[i][j] >= 0) ret[i][j] += delta;
                    else ret[i][j] = delta;
                }
                else if(delta <= EPS) {
                    if(prevX[i][j] <= 0) ret[i][j] += delta;
                    else ; // ignored
                }
                
                if(ret[i][j] == 0) {
                    if(!nullified) nullified = true;
                    else ret[i][j] = EPS;
                }
            }
        }
    }
    
    return ret;
};

static Matrix
advance_d(
    Matrix const& prevD,
    Matrix const& X, 
    Cell const& mn
) {
    Vector2D<bool> notZero (X.size(), vector<bool>(X[0].size()));
    for(auto i = 0u; i < X.size(); ++i) {
        for(auto j = 0u; j < X[0].size(); ++j) {
            if(X[i][j] != 0) notZero[i][j] = true;
        }
    }
    vector<bool> hstroke (X.size());
    vector<bool> vstroke (X[0].size());
    for(auto i = 0u; i < X[0].size(); ++i) {
        hstroke[mn.first] = true;
    }
    
    unsigned mi = mn.first;
    unsigned mj = mn.second;
    
    bool needStriking = true;
    int  direction = 0;
    while(needStriking) {
        needStriking = false;
        
        if(even(direction)) { // vertical striking
            for(auto i = 0u; i < X.size(); ++i) {
                for(auto j = 0u; j < X[0].size(); ++j) {
                    if(i == mi && j == mj) continue;
                    
                    if(hstroke[i] && !vstroke[j] && notZero[i][j]) {
                        vstroke[j] = true;
                        needStriking = true;
                    }
                }
            }
        }
        else { // horizontal striking
            for(auto j = 0u; j < X[0].size(); ++j) {
                for(auto i = 0u; i < X.size(); ++i) {
                    if(i == mi && j == mj) continue;
                    
                    if(vstroke[j] && !hstroke[i] && notZero[i][j]) {
                        hstroke[i] = true;
                        needStriking = true;
                    }
                }
            }
        }
        
        ++direction;
    }
    
    Matrix ret = prevD;
    
    auto delta = std::abs(prevD[mn.first][mn.second]);
    for(auto i = 0u; i < X.size(); ++i) {
        for(auto j = 0u; j < X[0].size(); ++j) {
            if(hstroke[i] && !vstroke[j]) ret[i][j] += delta;
            else if(vstroke[j] && !hstroke[i]) ret[i][j] -= delta;
        }
    }
    
    return ret;
}

static void calculate_w(BalanceMatrix::Step& step, Matrix const& costs) {
    for(auto i = 0u; i < costs.size(); ++i) {
        for(auto j = 0u; j < costs[0].size(); ++j) {
            if(step.X[i][j] > 0) {
                step.W += costs[i][j] * step.X[i][j];
            }
        }
    }
}

auto BalanceMatrix::solve(Meth const& m) const -> vector<Step> {
    if(_costs.empty()) return {};
    
    /*setup*/
    vector<int> consums;
    vector<int> prods;
    Matrix costs;
    Step s;
    if(m == Meth::NW) {
        s.X = get_key0_by_nw_method(&consums, &prods, &costs);
    }
    else {
        s.X = get_key0_by_min_method(&consums, &prods, &costs);
    }
    
    vector<int> U, V;
    fill_uv(costs, s.X, U, V);
    
    assert(U.size() == consums.size());
    assert(V.size() == prods.size());
    
    s.D = get_price0(costs, U, V);
    
    calculate_w(s, costs);
    vector<Step> ret {s};
    
    while(!all_positive(s.D)) {
        auto mn = most_negative_element(s.D);
        
        Step ns;
        ns.X = advance_x(s.X, mn);
        ns.D = advance_d(s.D, ns.X, mn);
        calculate_w(ns, costs);
        ret.push_back(ns);
        s = ns;
    }
    
    return ret;
}

bool BalanceMatrix::Step::valid() const {
    return all_positive(D);
}
