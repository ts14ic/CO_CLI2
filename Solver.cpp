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

std::ostream& operator<<(std::ostream& os, Solver const& solver) {
    if(!std::ostream::sentry{os}) return os;
    
    
    os << "[Solver\n";
    auto const& goal = solver._goal;
    if(goal.size() != 0) {
        os << goal.right() << ':';
        for(int i : goal.indices()) {
            os << ' ' << std::setw(2) << goal.coeff(i) << (goal.big(i) ? "M" : "");
        }
        os << '\n';
    }
    for(Restriction const& r : solver._restrs) {
        os << std::setw(4) << r.right();
        for(auto i : r.indices()) {
            os << ' ' << std::setw(2) << r.coeff(i);
        }
        os << " " << r.rel() << '\n';
    }
    os << "]";
    
    return os;
}

bool Solver::set_goal(std::string const& str) {
    Goal newGoal;
    if(!newGoal.parse_and_set(str)) return false;
    
    _initialBasis = newGoal.indices();
    _goal = newGoal;
    
    return true;
}



bool Solver::add_restriction(std::string const& str) {
    Restriction newRestriction;
    if(!newRestriction.parse_and_set(str)) return false;
    
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
    auto termsNum = _goal.size();
    auto restrNum = _restrs.size();
    _sel.resize(restrNum);
    
    for(auto i = 1; i <= termsNum; ++i) {
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

static void calculate_wm(Solver::Step& step) {
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

static void calculate_price(Solver::Step& s) {
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

static bool need_to_calc_artificial(Solver::Step const& s) {
    bool ret = false;
    
    for(int i : s.goal.indices()) {
        if(s.goal.big(i)) {
            ret = true;
            break;
        }
    }
    
    return ret;
}

static int max_element(Polynom const& g) {
    int ret = g.last_idx();
    for(int i : g.indices()) {
        if(g.coeff(i) > g.coeff(ret)) ret = i;
    }
    return ret;
}

static int min_element(Polynom const& g) {
    int ret = g.last_idx();
    for(int i : g.indices()) {
        if(g.coeff(i) < g.coeff(ret)) ret = i;
    }
    return ret;
}

static int select_column(Solver::Step& s, bool artificial) {
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

static void pack_end_results(Solver::Step& lastStep, vector<int> const& indices) {
    auto restrsNum = lastStep.restrs.size();
    
    for(int i : indices) {
        bool present = false;
        for(auto row = 0u; row < restrsNum; ++row) {
            if(i == lastStep.sel[row].idx()) {
                Term t{i, lastStep.restrs[row].right()};
                lastStep.basis.push_back(t);
                present = true;
                break;
            }
        }
        if(!present) {
            lastStep.basis.push_back(Term{i});
        }
    }
}

static vector<Optional<Fraction>>::const_iterator 
min_div(vector<Optional<Fraction>> const& divs) {
    // first we need to find any allowed value
    auto i = divs.begin();
    while(i != divs.end()) {
        if(*i) break;
        ++i;
    }
    // it's possible, there is no min allowed value
    if(i == divs.end()) return i;
    // second, if any values remaining, we look if there are smaller values
    bool smallestIsUniq = true;
    auto smallest = i;
    ++i;
    while(i != divs.end()) {
        if(*i) {
            if(**i < **smallest) {
                smallest = i;
                smallestIsUniq = true;
            }
            else if(**i == **smallest){
                smallestIsUniq = false;
            }
        }
        ++i;
    }
    
    if(smallestIsUniq) {
        return smallest;
    }
    else {
        return divs.end();
    }
}

static unsigned select_row(vector<Fraction> const& selCol, vector<Optional<Fraction>> const& divndCol) {
    auto const rowsNum = selCol.size();
    
    vector<Optional<Fraction>> divs (rowsNum);
    bool noValues = true;
    
    for(auto i = 0u; i < rowsNum; ++i) {
        // if there is what to divide, and there is no division by zero
        if(divndCol[i] && selCol[i] != 0) {
            noValues = false;
            auto div = *divndCol[i] / selCol[i];
            if(div >= 0) divs[i] = div;
        }
    }
    
    auto minDiv = min_div(divs);
    if(minDiv != divs.end()) {
        return std::distance(divs.cbegin(), minDiv);
    }
    else if(noValues) {
        return rowsNum;
    }
    else {
        return select_row(selCol, divs);
    }
}

static unsigned select_row(Solver::Step const& s, int col) {
    auto rowsNum = s.restrs.size();
    
    vector<Fraction>           selCol (rowsNum);
    vector<Optional<Fraction>> divnd (rowsNum);
    
    for(auto i = 0u; i < rowsNum; ++i) {
        selCol[i] = s.restrs[i].coeff(col);
        divnd[i]  = s.restrs[i].right();
    }
    
    return select_row(selCol, divnd);
}

static Solver::Step advance_step(Solver::Step const& prev, int selCol, unsigned selRow) {
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
        steps.push_back(s);
        
        int selCol = 0;
        if(need_to_calc_artificial(s)) {
            selCol = select_column(s, true);
        }
        // if still no column
        if(selCol == 0) {
            selCol = select_column(s, false);
        }
        
        // if again no column selected, it's finish
        if(selCol == 0) {
            pack_end_results(steps.back(), _initialBasis);
            break;
        }
        
        auto selRow = select_row(s, selCol);
        // another finish, but this time it's unsolvable 100%
        if(selRow == _restrs.size()) {
            steps.back().basis.clear();
            break;
        }
        
        // strip out M columns, switch selected rows and calculate new table
        s = advance_step(s, selCol, selRow);
    }
    
    return steps;
}

bool Solver::Step::valid() const {
    if(basis.size() == 0) return false;
    
    for(auto const& t : goal.terms()) {
        if(t.big()) return false;
    }
    return true;
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
