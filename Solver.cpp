#include "Solver.h"
#include "Goal.h"
#include "Restriction.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <boost/optional.hpp>

using std::vector;

template <typename T>
using Optional = boost::optional<T>;

using OptFraction = boost::optional<Fraction>;

std::ostream& operator<<(std::ostream& os, Solver const& solver) {
    if(!std::ostream::sentry{os}) return os;
    
    os << "[Solver\n";
    auto const& goal = solver._goal;
    if(goal.size() != 0) {
        os << goal.right() << ':';
        for(int i : goal.indices()) {
            os << std::setw(goal.big(i) ? 3 : 4) <<
                  goal.coeff(i) << (goal.big(i) ? "M" : "");
        }
        os << '\n';
    }
    for(Restriction const& r : solver._restrs) {
        os << std::setw(4) << r.right();
        for(auto i : r.indices()) {
            os << std::setw(4) << r.coeff(i);
        }
        os << " " << r.rel() << '\n';
    }
    os << "]";
    
    return os;
}

bool Solver::set_goal(std::string const& str) {
    if(!_restrs.empty()) return false;
    
    Goal newGoal;
    if(!newGoal.parse_and_set(str)) return false;
    
    _initialBasis = newGoal.indices();
    _goal = newGoal;
    
    return true;
}



bool Solver::add_restriction(std::string const& str) {
    if(_goal.terms().empty()) return false;
    
    Restriction newRestriction;
    if(!newRestriction.parse_and_set(str)) return false;
    
    if(_goal.size() > newRestriction.size()) {
        newRestriction.add_term(_goal.last_idx());
    }
    else if(_goal.size() < newRestriction.size()) {
        _goal.add_term(newRestriction.last_idx());
        _initialBasis = _goal.indices();
        
        for(auto& res : _restrs) {
            if(res.size() < newRestriction.size()) {
                res.add_term(newRestriction.last_idx());
            }
        }
    }
    
    _restrs.push_back(newRestriction);
    return true;
}

void Solver::append_preferred() {
    for(auto& r : _restrs) {
        if(r.rel() != "==") {
            r.add_term(
                r.next_idx(),
                r.rel() == "<=" ? 1 : -1
            );
            
            for(auto& sr : _restrs) {
                if(&sr == &r) continue;
                sr.add_term(sr.next_idx());
            }
            _goal.add_term(_goal.next_idx());
            
            r.rel("==");
        }
    }
}

void Solver::append_artificial() {
    auto restrNum = _restrs.size();
    _sel.resize(restrNum);
    
    for(int i : _goal.indices()) {
        bool gotOne = false;
        
        for(auto r = 0u; r < restrNum; ++r) {
            auto coeff = _restrs[r].coeff(i);
            if(coeff == 0) continue;
            if(coeff == 1) {
                if(!gotOne) {
                    gotOne = true;
                    _sel[r] = _goal.term(i);
                }
                else {
                    _sel[r] = {0};
                    break;
                }
            }
            else {
                _sel[r] = {0};
                break;
            }
        }
    }
    
    for(auto r = 0u; r < restrNum; ++r) {
        if(_sel[r] == Term{0}) {
            int newIdx = _restrs[r].next_idx();
        
            _restrs[r].add_term(newIdx, 1);
            
            Fraction newCoeff = (_goal.right() == "min" ? 1 : -1);
            _goal.add_term(newIdx, newCoeff, true);
            
            for(auto& restr : _restrs) {
                if(&_restrs[r] == &restr) continue;
                restr.add_term(newIdx, 0);
            }
            
            _sel[r] = _goal.term(newIdx);
        }
    }
}

Solver& Solver::invert_to_dual() {
    // FIXME check if restrictions are flat

    int oldTermsNum = _goal.size();
    int oldRestrNum = _restrs.size();
    
    // first, see what restrictions would need their sign changed
    std::string fromRel, toRel, toDir;
    if(_goal.right() == "min") {
        fromRel = "<=";
        toRel   = ">=";
        toDir   = "max";
    }
    else {
        fromRel = ">=";
        toRel   = "<=";
        toDir   = "min";
    }
    
    // change original restrictions sign
    for(auto& r: _restrs) {
        if(r.rel() == fromRel) {
            for(int j : r.indices()) {
                r.coeff(j) = -r.coeff(j);
            }
            r.right() = -r.right();
        }
    }
    
    Goal newGoal;
    newGoal.right(toDir);
    vector<Restriction> newRestrs (oldTermsNum);
    
    // form new goal and new restrictions
    for(auto i = 0; i < oldRestrNum; ++i) {
        newGoal.add_term(i + 1, _restrs[i].right());
        
        for(auto j = 0; j < oldTermsNum; ++j) {
            Term const& t = _restrs[i].term(j + 1);
            newRestrs[j].add_term(i + 1, t.coeff());
        }
    }
    
    // set new restrictions relation and fix negative free terms
    for(auto i = 0; i < oldTermsNum; ++i) {
        newRestrs[i].rel(fromRel);
        
        // if a negative row is created, fix it
        if(_goal.coeff(i + 1) < 0) {
            newRestrs[i].right() = - _goal.coeff(i + 1);
            for(int j : newRestrs[i].indices()) {
                newRestrs[i].coeff(j) = -newRestrs[i].coeff(j);
            }
            newRestrs[i].rel(toRel);
        }
        else {
            newRestrs[i].rel(fromRel);
            newRestrs[i].right() = _goal.coeff(i + 1);
        }
    }
    
    // fix initial basis also
    _initialBasis = newGoal.indices();
    
    _goal = newGoal;
    _restrs = newRestrs;
    return *this;
}

inline namespace helpers {
    void 
    calculate_wm(Solver::Step& step) {
        auto restrsNum = step.restrs.size();
        
        step.w = 0;
        step.m = 0;
        for(auto row = 0u; row < restrsNum; ++row) {
            if(!step.sel[row].big()) {
                step.w += step.sel[row].coeff() * step.restrs[row].right();
            }
            else {
                step.m += step.sel[row].coeff() * step.restrs[row].right();
            }
        }
    }

    void 
    calculate_price(Solver::Step& s) {
        auto restrsNum = s.restrs.size();
        
        for(int col : s.goal.indices()) {
            Fraction psum, msum;
            for(auto row = 0u; row < restrsNum; ++row) {
                auto& resTerms = s.restrs[row];
                
                auto toAdd = s.sel[row].coeff() * resTerms.coeff(col);
                
                if(s.sel[row].big()) {
                    msum += toAdd;
                }
                else {
                    psum += toAdd;
                }
            }
            
            Term const& colTerm = s.goal.term(col);
            if(colTerm.big()) {
                msum -= colTerm.coeff();
            }
            else {
                psum -= colTerm.coeff();
            }
            
            s.pprice.coeff(col) = psum;
            s.mprice.coeff(col) = msum;
        }
        
        calculate_wm(s);
    }

    bool 
    need_to_calc_artificial(Solver::Step const& s) {
        bool ret = false;
        
        for(int i : s.goal.indices()) {
            if(s.goal.big(i)) {
                ret = true;
                break;
            }
        }
        
        return ret;
    }

    int 
    max_element(Polynom const& g) {
        int ret = g.last_idx();
        for(int i : g.indices()) {
            if(g.coeff(i) > g.coeff(ret)) ret = i;
        }
        return ret;
    }

    int 
    min_element(Polynom const& g) {
        int ret = g.last_idx();
        for(int i : g.indices()) {
            if(g.coeff(i) < g.coeff(ret)) ret = i;
        }
        return ret;
    }

    int
    select_column(Solver::Step& s, bool artificial) {
        Polynom const& pol = artificial ? s.mprice : s.pprice;
        
        if(s.goal.right() == "min") {
            int selCol = max_element(pol);
            if(pol.coeff(selCol) > 0) return selCol;
        }
        else {
            int selCol = min_element(pol);
            if(pol.coeff(selCol) < 0) return selCol;
        }
        
        return 0;
    }

    void 
    pack_end_results(Solver::Step& lastStep, vector<int> const& indices) {
        auto restrsNum = lastStep.restrs.size();
        
        for(int i : indices) {
            bool selected = false;
            for(auto row = 0u; row < restrsNum; ++row) {
                if(i == lastStep.sel[row].idx()) {
                    Term t{i, lastStep.restrs[row].right()};
                    lastStep.basis.push_back(t);
                    selected = true;
                    break;
                }
            }
            if(!selected) {
                lastStep.basis.push_back(Term{i});
            }
        }
        
        lastStep.mark_as_valid();
    }
    
    vector<Fraction>
    get_col(Solver::Step const& s, int col) {
        auto const rowsNum = s.restrs.size();
        
        vector<Fraction> ret (rowsNum);
        for(auto row = 0u; row < rowsNum; ++row) {
            if(col == 0) {
                ret[row] = s.restrs[row].right();
            }
            else {
                ret[row] = s.restrs[row].coeff(col);
            }
        }
        
        return ret;
    }
    
    enum class Division {
        DontAllowNegative,
        AllowNegative
    };
    
    vector<OptFraction>
    divide_cols(vector<Fraction> const& a, vector<Fraction> const& b, Division const& policy) {
        auto const rowsNum = a.size();
        
        vector<OptFraction> ret (rowsNum);
        
        for(auto i = 0u; i < rowsNum; ++i) {
            if((policy == Division::DontAllowNegative && b[i] > 0) || 
               (policy == Division::AllowNegative && b[i] != 0)) 
            {
                ret[i] = a[i] / b[i];
            }
        }
        
        return ret;
    }
    
    vector<unsigned>
    get_indices_for_min(vector<OptFraction> const& range) {
        auto const rowsNum = range.size();
        
        auto it = range.cbegin();
        while(!*it && it != range.cend()) {
            ++it;
        }
        
        if(it == range.cend()) {
            return {};
        }
        
        auto smallest = it;
        for(++it; it != range.cend(); ++it) {
            if(*it && **it < **smallest) smallest = it;
        }
        
        vector<unsigned> ret;
        for(auto i = 0u; i < rowsNum; ++i) {
            if(range[i] && *range[i] == **smallest) ret.push_back(i);
        }
        return ret;
    }
    
    vector<unsigned>
    get_indices_for_min(vector<OptFraction> const& range, vector<unsigned> const& indicesToCheck) {
        auto const rowsNum = range.size();
        
        auto smallest = indicesToCheck.empty() ? 0u : indicesToCheck.front();
        for(auto i : indicesToCheck) {
            if(*range[i] < *range[smallest]) smallest = i;
        }
        
        vector<unsigned> ret;
        for(auto i = 0u; i < rowsNum; ++i) {
            if(*range[i] == *range[smallest]) ret.push_back(i);
        }
        return ret;
    }

    unsigned 
    select_row(Solver::Step const& s, int col) {
        auto rowsNum = s.restrs.size();
        
        auto dividend = get_col(s, 0);
        auto const divisor = get_col(s, col);
        
        auto divs = divide_cols(dividend, divisor, Division::DontAllowNegative);
        auto const initialIndices = get_indices_for_min(divs);
        
        if(initialIndices.size() == 1) {
            return initialIndices.front();
        }
        
        for(int i : s.goal.indices()) {
            if(i == col) continue;
            
            dividend = get_col(s, i);
            divs = divide_cols(dividend, divisor, Division::AllowNegative);
            
            auto minRows = get_indices_for_min(divs, initialIndices);
            
            if(minRows.size() == 1) {
                return minRows.front();
            }
        }
        
        return rowsNum;
    }
    
    Solver::Step 
    advance_step(Solver::Step const& prev, int selCol, unsigned selRow) {
        Solver::Step next = prev;
        
        if(next.goal.big(prev.sel[selRow].idx())) {
            next.goal.remove_term(prev.sel[selRow].idx());
            for(auto& r: next.restrs) {
                r.remove_term(prev.sel[selRow].idx());
            }
            
            auto idx = prev.sel[selRow].idx();
            next.pprice.remove_term(idx);
            next.mprice.remove_term(idx);
        }
        next.sel[selRow] = next.goal.term(selCol);
        
        auto restrsNum = prev.restrs.size();
        auto divisor = prev.restrs[selRow].coeff(selCol);
            
        for(auto r = 0u; r < restrsNum; ++r) {
            if(r == selRow) {
                for(int i : next.goal.indices()) {
                    next.restrs[r].coeff(i) /= divisor;
                }
                next.restrs[r].right() /= divisor;
            }
            else {
                for(int i : next.goal.indices()) {
                    next.restrs[r].coeff(i) =
                        (prev.restrs[selRow].coeff(selCol) * prev.restrs[r].coeff(i) -
                        prev.restrs[r].coeff(selCol) * prev.restrs[selRow].coeff(i)) /
                        divisor;
                }
                
                next.restrs[r].right() =
                    (prev.restrs[selRow].coeff(selCol) * prev.restrs[r].right() -
                    prev.restrs[r].coeff(selCol) * prev.restrs[selRow].right())
                    / divisor;
            }
        }
        
        return next;
    }

    bool 
    step_is_unique(Solver::Step const& step, vector<Solver::Step> const& steps) {
        for(auto const& s : steps) {
            if(step == s) {
                return false;
            }
        }
        return true;
    }
}



vector<Solver::Step> Solver::solve() {
    append_preferred();
    append_artificial();
    
    // setup
    vector<Step> steps;
    Step s;
    s.goal   = _goal;
    s.sel    = _sel;
    s.restrs = _restrs;
    s.pprice.add_term(_goal.last_idx());
    s.mprice.add_term(_goal.last_idx());
    
    while(true) {
        calculate_price(s);
        // if the step repeats itself, it's unsolvable
        if(!step_is_unique(s, steps)) {
            steps.push_back(s);
            break;
        }
        else {
            steps.push_back(s);
        }
        
        int selCol = 0;
        if(need_to_calc_artificial(s)) {
            selCol = select_column(s, true);
        }
        // if still no column or no need to calc artificial
        if(selCol == 0) {
            selCol = select_column(s, false);
        }
        // if again no column selected, it's a finish
        if(selCol == 0) {
            pack_end_results(steps.back(), _initialBasis);
            break;
        }
        
        auto selRow = select_row(s, selCol);
        // if no row selected, it's unsolvable
        if(selRow == _restrs.size()) {
            break;
        }
        
        // strip out M columns, switch selected rows and calculate new table
        s = advance_step(s, selCol, selRow);
    }
    
    return steps;
}

bool Solver::Step::valid() const {
    for(auto const& term : goal.terms()) {
        if(term.big()) return false;
    }
    
    return _valid;
}

bool Solver::Step::operator ==(Step const& o) const {
    return goal == o.goal &&
           sel == o.sel &&
           restrs == o.restrs;
}

void Solver::Step::mark_as_valid() {
    _valid = true;
}

void print_step(std::ostream& os, Solver::Step const& s, bool price, bool newline) {
    auto tab = "   ";
    
    os << "<Step>\n";
    os << tab << "<Goal>" << s.goal << "</Goal>\n";
    os << tab << "<Restrs>\n";
    for(auto i = 0u; i < s.restrs.size(); ++i) {
        os << tab << tab << s.restrs[i] << " " << s.sel[i] << "\n";
    }
    
    if(!price) {
        os << tab << "</Restrs>\n</Step>\n";
        if(newline) os << "\n";
        return;
    }
    
    os << tab << "</Restrs>\n" << tab << "<pprice>";
    for(int i : s.pprice.indices()) {
        os << std::setw(4) << s.pprice.coeff(i);
    }
    os << "</pprice>\n" << tab << "<mprice>";
    for(int i : s.mprice.indices()) {
        os << std::setw(4) << s.mprice.coeff(i);
    }
    os << "</mprice>\n</Step>\n";
    
    if(newline) os << '\n';
}
