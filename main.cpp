#include "Solver.h"

#include <UnitTest++/UnitTest++.h>

#include <iostream>
#include <sstream>

SUITE(Fraction) {
    TEST(FractionInitialization) {
        // default constructor gives 0 (zero)
        Fraction a;
        CHECK(a.num() == 0);
        CHECK(a.den() == 1);
        
        // implicit denominator is 1 again
        Fraction b {10};
        CHECK(b.num() == 10);
        CHECK(b.den() == 1);
        
        // full constructor, all positive
        Fraction c {3, 4};
        CHECK(c.num() == 3);
        CHECK(c.den() == 4);
        
        // full constructor, nominator remains negative
        Fraction d (-3, 2);
        CHECK(d.num() == -3);
        CHECK(d.den() == 2);
        
        // full constructor, propagate negativity from denom to nom
        Fraction e (3, -2);
        CHECK(e.num() == -3);
        CHECK(e.den() == 2);
        
        // full constructor, both negative
        Fraction f(-5, -11);
        CHECK(f.num() == 5);
        CHECK(f.den() == 11);
        
        CHECK_THROW(Fraction(42, 0), std::exception);
    }
    
    TEST(FractionOperations) {
        Fraction a{2, 3};
        a = a * 3 / 2;
        CHECK(a == Fraction(1));
        
        a = (a + 2) / 2 - 1;
        CHECK(a == Fraction(1, 2));
        
        a = a * Fraction(5, 4) - Fraction(1, 4) + Fraction(3, 2);
        CHECK(a == Fraction(15, 8));
        
        a += Fraction(1);
        CHECK(Fraction(23, 8) == a);
        
        a -= Fraction(13, 8);
        CHECK(Fraction(5, 4) == a);
        
        a *= Fraction(2, 3);
        CHECK(Fraction(5, 6) == a);
        
        a += 1;
        CHECK(Fraction(11, 6) == a);
    }
    
    TEST(FractionComparisons) {
        CHECK(Fraction(10) == Fraction(10));
        CHECK(Fraction(2, 6) == Fraction(2, 6));
        CHECK(Fraction(2, 6) == Fraction(1, 3));
        
        CHECK(Fraction(3, 3) == 1);
        CHECK(Fraction(3, 4) != 1);
        
        CHECK(Fraction(2) != Fraction(3));
        CHECK(Fraction(2) != 3);
        CHECK(Fraction(2, 6) != Fraction(1, 6));
        
        CHECK(Fraction(3, 2) > Fraction(1));
        CHECK(Fraction(3, 2) > 1);
        CHECK(Fraction(10, 5) < Fraction(10, 3));
        CHECK(Fraction(10, 5) <= Fraction(10, 3));
        CHECK(Fraction(10, 5) <= Fraction(10, 5));
    }
}


SUITE(Term) {
    TEST(TermInitialization) {
        std::stringstream ss;
        
        Term a(10);
        ss << a;
        CHECK(ss.str() == "0{X10}");
        ss.str("");
        
        Term d(3, Fraction(1, 5));
        ss << d;
        CHECK(ss.str() == "1/5{X3}");
        ss.str("");
        
        Term b(3, Fraction(2, 3));
        ss << b;
        CHECK(ss.str() == "2/3{X3}");
        ss.str("");
        
        Term coeff(3, true);
        ss << coeff;
        CHECK(ss.str() == "0M{X3}");
        ss.str("");
        
        Term e(5, Fraction(1, 3), true);
        ss << e;
        CHECK(ss.str() == "1/3M{X5}");
    }
}


SUITE(Polynom) {
    TEST(PolynomParsing) {
        Polynom p;
        std::stringstream ss;
        
        CHECK(p.parse_and_set("2X1 - x2"));
        CHECK(p.size() == 2);
        ss << p;
        CHECK(ss.str() == "[Polynom: 2{X1} -1{X2}]");
        
        CHECK(p.parse_and_set("10X2"));
        CHECK(p.size() == 2);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 0{X1} 10{X2}]");
        
        CHECK(p.parse_and_set("-5X1 + 2X3"));
        CHECK(p.size() == 3);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: -5{X1} 0{X2} 2{X3}]");
        
        CHECK(!p.parse_and_set("2X1 - X"));
        CHECK(!p.parse_and_set("2X1 -"));
        CHECK(!p.parse_and_set("X1 + 10"));

        CHECK(ss.str() == "[Polynom: -5{X1} 0{X2} 2{X3}]");
    }
    
    TEST(PolynomTermGetSetting) {
        Polynom p;
        std::stringstream ss;
        
        p.parse_and_set("X1 + 3X2 -X3 + 3X4");
        p.coeff(3) = Fraction(10, 15);
        p.coeff(1) = Fraction(4, 7);
        // setting non-existent terms throws exceptions
        CHECK_THROW((p.coeff(5) = Fraction(3, 4)), std::range_error);
        
        CHECK(p.coeff(1) == Fraction(4, 7));
        CHECK(p.coeff(2) == 3);
        CHECK(p.coeff(3) == Fraction(2, 3));
        CHECK(p.coeff(4) == 3);
        // accessing non-existent coeffs also throws
        CHECK_THROW((p.coeff(5) == 0), std::range_error);
        CHECK_THROW((p.coeff(0) == 0), std::range_error);
        
        p.coeff(2) = Fraction(1, 3);
        CHECK(p.coeff(2) == Fraction(1, 3));
        p.coeff(2) = p.coeff(1) * 3/2;
        CHECK(p.coeff(2) == Fraction(6, 7));
    }
    
    TEST(PolynomTermAdding) {
        std::stringstream ss;
        Polynom p;
        
        // one can add a term
        p.add_term(1, Fraction(2, 3));
        CHECK(p.size() == 1);
        ss << p;
        CHECK(ss.str() == "[Polynom: 2/3{X1}]");
        
        // simillar terms are summed
        p.add_term(1, Fraction(1, 3));
        CHECK(p.size() == 1);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 1{X1}]");
        
        // even with other terms in the way
        p.add_term(2, Fraction(4, 5));
        p.add_term(1, Fraction(3, 4));
        CHECK(p.size() == 2);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 7/4{X1} 4/5{X2}]");
        
        // terms are sorted
        p.add_term(4, Fraction(1, 8));
        CHECK(p.size() == 4);
        p.add_term(3, Fraction(2, 4));
        CHECK(p.size() == 4);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 7/4{X1} 4/5{X2} 1/2{X3} 1/8{X4}]");
        
        // and gaps are filled
        p.clear_terms();
        CHECK(p.size() == 0);
        p.add_term(4, Fraction(3, 7));
        CHECK(p.size() == 4);
        p.add_term(2, Fraction(3, 5));
        CHECK(p.size() == 4);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 0{X1} 3/5{X2} 0{X3} 3/7{X4}]");
        
        // you can add M terms only after parsing
        p.remove_term(3);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 0{X1} 3/5{X2} 3/7{X4}]");
        
        p.add_term(3, Fraction{1, 3}, true);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 0{X1} 3/5{X2} 1/3M{X3} 3/7{X4}]");
    }
    
    TEST(PolynomTermRemoving) {
        Polynom p;
        std::stringstream ss;
        
        p.parse_and_set("X1 + 3X2 -X3 + 3X4");
        CHECK(p.size() == 4);
        p.remove_term(1);
        CHECK(p.size() == 3);
        ss << p;
        CHECK(ss.str() == "[Polynom: 3{X2} -1{X3} 3{X4}]");
        
        p.remove_term(3);
        CHECK(p.size() == 2);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 3{X2} 3{X4}]");
        
        // can't delete what doesn't even exist
        p.remove_term(5);
        CHECK(p.size() == 2);
        ss.str(""); ss << p;
        CHECK(ss.str() == "[Polynom: 3{X2} 3{X4}]");
    }
}


SUITE(GoalPolynom) {
    TEST(GoalInitialization) {
        std::stringstream ss;
        Goal g;

        CHECK(g.parse_and_set("x2 + 3x4 => max"));
        ss << g;
        CHECK(ss.str() == "[Goal: 0{X1} 1{X2} 0{X3} 3{X4} => max]");
        
        CHECK(g.parse_and_set("-4x1 + 5x3 => max"));
        ss.str(""); ss << g;
        CHECK(ss.str() == "[Goal: -4{X1} 0{X2} 5{X3} => max]");
        
        CHECK(g.parse_and_set("3x1 - 5x2 + 0x3 => min"));
        ss.str(""); ss << g;
        CHECK(ss.str() == "[Goal: 3{X1} -5{X2} 0{X3} => min]");
        
        CHECK(!g.parse_and_set("3x1 - 5x2 + 0x3 > min"));
        CHECK(!g.parse_and_set("3x1 - 5x2 + 0x3 >> max"));
        CHECK(!g.parse_and_set("3x1 - 5x2 + 0x3 >= min"));
    }
    
    
    TEST(GoalAccessors) {
        std::stringstream ss;
        Goal g;
        
        CHECK(g.parse_and_set("x2 + 3x4 => max"));
        g.remove_term(1);
        g.remove_term(3);
        CHECK(g.size() == 2);
        ss << g;
        CHECK(ss.str() == "[Goal: 1{X2} 3{X4} => max]");
        
        g.add_term(1, Fraction(2, 3));
        ss.str(""); ss << g;
        CHECK(ss.str() == "[Goal: 2/3{X1} 1{X2} 0{X3} 3{X4} => max]");
        
        g.add_term(1);
        ss.str(""); ss << g;
        CHECK(ss.str() == "[Goal: 2/3{X1} 1{X2} 0{X3} 3{X4} => max]");
        
        g.add_term(5, 1, true);
        ss.str(""); ss << g;
        CHECK(ss.str() == "[Goal: 2/3{X1} 1{X2} 0{X3} 3{X4} 1M{X5} => max]");
        
        CHECK(g.right() == "max");
        g.right("min");
        CHECK(g.right() == "min");
        g.right("chewy");
        CHECK(g.right() == "min");
    }
}


SUITE(RestrictionPolynom) {
    TEST(RestrictionPolynomInitialization) {
        std::stringstream ss;

        Restriction r0;
        CHECK(r0.parse_and_set("x2 + 3x4 <= 16"));
        ss << r0;
        CHECK(ss.str() == "[Restriction: 0{X1} 1{X2} 0{X3} 3{X4} <= 16]");
        ss.str("");
        
        Restriction r1;
        CHECK(r1.parse_and_set("-4x1 + 5x3 == 10"));
        ss << r1;
        CHECK(ss.str() == "[Restriction: -4{X1} 0{X2} 5{X3} == 10]");
        ss.str("");
        
        Restriction r2;
        CHECK(r2.parse_and_set("3x1 - 5x2 + 0x3 >= -14"));
        ss << r2;
        CHECK(ss.str() == "[Restriction: 3{X1} -5{X2} 0{X3} >= -14]");
        ss.str("");
        
        Restriction r3;
        CHECK(!r3.parse_and_set("3x1 - 5x2 + 0x3 > -14"));
        CHECK(!r3.parse_and_set("3x1 - 5x2 + 0x3 >> 10"));
        CHECK(!r3.parse_and_set("3x1 - 5x2 + 0x3 >="));
    }
    
    TEST(RestrictionAccessors) {
        std::stringstream ss;
        Restriction r;
        
        CHECK(r.parse_and_set("x2 + 3x4 <= 16"));
        r.remove_term(1);
        r.remove_term(3);
        CHECK(r.size() == 2);
        ss << r;
        CHECK(ss.str() == "[Restriction: 1{X2} 3{X4} <= 16]");
        
        r.add_term(1, Fraction(2, 3));
        ss.str(""); ss << r;
        CHECK(ss.str() == "[Restriction: 2/3{X1} 1{X2} 0{X3} 3{X4} <= 16]");
        
        r.rel("--->");
        CHECK(r.rel() == "<=");
        r.rel("==");
        CHECK(r.rel() == "==");
        
        r.right() = Fraction(2);
        CHECK(r.right() == 2);
        r.right() += Fraction(1, 2);
        CHECK(r.right() == Fraction(5, 2));
    }
}


SUITE(Solver) {
    struct SolverFixture {
        SolverFixture() {
            solver[0].set_goal("x1 + x2 => min");
            solver[0].add_restriction(" 2x1 + 4x2 <= 16");
            solver[0].add_restriction("-4x1 + 2x2 <=  8");
            solver[0].add_restriction(" 1x1 + 3x2 >=  9");
            
            solver[1].set_goal("7x1 - 2x2 => min");
            solver[1].add_restriction("5x1 - 2x2 <= 3");
            solver[1].add_restriction(" x1 +  x2 >= 1");
            solver[1].add_restriction("2x1 +  x2 <= 4");
            
            solver[2].set_goal("2x1 +  3x2 => min");
            solver[2].add_restriction(" 2x1 +  x2 <= 10");
            solver[2].add_restriction("-2x1 + 3x2 <=  6");
            solver[2].add_restriction(" 2x1 + 4x2 >=  8");
            
            // problem has no solution
            solver[3].set_goal("2x1 + 7x2 => max");
            solver[3].add_restriction("12x1 + 13x2 <= 17");
            solver[3].add_restriction(" 3x1 +   x2 <=  5");
            solver[3].add_restriction("  x1 +  4x2 >=  6");
            
            solver[4].set_goal("4x1 + x2 => max");
            solver[4].add_restriction("2x1 -  x2 <= 12");
            solver[4].add_restriction(" x1 + 3x2 <= 18");
            solver[4].add_restriction("2x1 + 5x2 >= 10");
            
            solver[5].set_goal("4x1 + x2 => min");
            solver[5].add_restriction("2x1 -  x2 <= 12");
            solver[5].add_restriction(" x1 + 3x2 <= 18");
            solver[5].add_restriction("2x1 + 5x2 >= 10");
            
            solver[6].set_goal("2x1 + x2 => min");
            solver[6].add_restriction(" 3x1 - 2x2 <= 12");
            solver[6].add_restriction("-1x1 + 2x2 <=  8");
            solver[6].add_restriction(" 2x1 + 3x2 >=  5");
            
            solver[7].set_goal(" 8x1 + 7x2 => max");
            solver[7].add_restriction("  x1 -  2x2 <= 12");
            solver[7].add_restriction(" 4x1 +   x2 <= 16");
            solver[7].add_restriction(" 5x1 +  5x2 >= 25");
            
            special.set_goal("x1 + x2 => min");
            special.add_restriction("2x1 + 4x2 <= 16");
            special.add_restriction("-4x1 + 2x2 <= 8");
            special.add_restriction("1x1 + 3x2 + 1x4 >= 9");
        }
    
        Solver solver[8];
        Solver special;
    };
    
    TEST(SolverSetup) {
        Solver solver;
        CHECK(solver.set_goal("x1 + x2 => min"));
        CHECK(solver.add_restriction("-4x1 + 2x2 <= 8"));
        CHECK(solver.add_restriction("1x1 + 3x2 >= 9"));
        CHECK(solver.add_restriction("2x1 + 4x2 <= 16"));
        CHECK(!solver.add_restriction(""));
        CHECK(!solver.add_restriction("not a restriction"));
        
        std::stringstream ss;
        ss << solver;
        CHECK(ss.str() == "[Solver\n"
                          "min:  1  1\n"
                          "   8 -4  2 <=\n"
                          "   9  1  3 >=\n"
                          "  16  2  4 <=\n]");
    }
    
    TEST_FIXTURE(SolverFixture, SolverSolutions) {
        auto s = solver[0].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == 3);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 3);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[1].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == -8);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 4);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[2].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == 6);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 2);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[3].solve().back();
        CHECK(!s.valid());
        
        s = solver[4].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == Fraction(240, 7));
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == Fraction(54, 7));
                break;
                
                case 2:
                CHECK(t.coeff() == Fraction(24, 7));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[5].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == 2);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 2);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[6].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == Fraction(5, 3));
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == Fraction(5, 3));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[7].solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 2);
        CHECK(s.w == 112);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 16);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = special.solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 4);
        CHECK(s.w == 0);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                case 2:
                case 3:
                CHECK(t.coeff() == 0);
                break;
                
                case 4:
                CHECK(t.coeff() == 9);
                break;
                
                default: CHECK(0 == 1);
            }
        }
    }
    
    TEST_FIXTURE(SolverFixture, Inversion) {
        std::stringstream ss;
        
        solver[0].invert_to_dual();
        ss << solver[0];
        CHECK(ss.str() == "[Solver\n"
                          "max: -16 -8  9\n"
                          "   1 -2  4  1 <=\n"
                          "   1 -4 -2  3 <=\n"
                          "]");
        
        solver[1].invert_to_dual();
        ss.str(""); ss << solver[1];
        CHECK(ss.str() == "[Solver\n"
                          "max: -3  1 -4\n"
                          "   7 -5  1 -2 <=\n"
                          "   2 -2 -1  1 >=\n"
                          "]");
        
        solver[2].invert_to_dual();
        ss.str(""); ss << solver[2];
        CHECK(ss.str() == "[Solver\n"
                          "max: -10 -6  8\n"
                          "   2 -2  2  2 <=\n"
                          "   3 -1 -3  4 <=\n"
                          "]");
        
        solver[3].invert_to_dual();
        ss.str(""); ss << solver[3];
        CHECK(ss.str() == "[Solver\n"
                          "min: 17  5 -6\n"
                          "   2 12  3 -1 >=\n"
                          "   7 13  1 -4 >=\n"
                          "]");
        
        solver[4].invert_to_dual();
        ss.str(""); ss << solver[4];
        CHECK(ss.str() == "[Solver\n"
                          "min: 12 18 -10\n"
                          "   4  2  1 -2 >=\n"
                          "   1 -1  3 -5 >=\n"
                          "]");
        
        solver[5].invert_to_dual();
        ss.str(""); ss << solver[5];
        CHECK(ss.str() == "[Solver\n"
                          "max: -12 -18 10\n"
                          "   4 -2 -1  2 <=\n"
                          "   1  1 -3  5 <=\n"
                          "]");
        
        solver[6].invert_to_dual();
        ss.str(""); ss << solver[6];
        CHECK(ss.str() == "[Solver\n"
                          "max: -12 -8  5\n"
                          "   2 -3  1  2 <=\n"
                          "   1  2 -2  3 <=\n"
                          "]");
        
        solver[7].invert_to_dual();
        ss.str(""); ss << solver[7];
        CHECK(ss.str() == "[Solver\n"
                          "min: 12 16 -25\n"
                          "   8  1  4 -5 >=\n"
                          "   7 -2  1 -5 >=\n"
                          "]");

        special.invert_to_dual();
        ss.str(""); ss << special;
        CHECK(ss.str() == "[Solver\n"
                          "max: -16 -8  9\n"
                          "   1 -2  4  1 <=\n"
                          "   1 -4 -2  3 <=\n"
                          "   0  0  0  0 <=\n"
                          "   0  0  0  1 <=\n"
                          "]");
    }
    
    TEST_FIXTURE(SolverFixture, SolvingInverted) {
        auto s = solver[0].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == 3);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 0);
                break;
                
                case 3:
                CHECK(t.coeff() == Fraction(1, 3));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[1].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == -8);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 0);
                break;
                
                case 3:
                CHECK(t.coeff() == 2);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[2].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == 6);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 0);
                break;
                
                case 3:
                CHECK(t.coeff() == Fraction(3, 4));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        // Unsolvable, success but slow
        s = solver[3].invert_to_dual().solve().back();
        CHECK(!s.valid());
        
        s = solver[4].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == Fraction(240, 7));
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == Fraction(11, 7));
                break;
                
                case 2:
                CHECK(t.coeff() == Fraction(6, 7));
                break;
                
                case 3:
                CHECK(t.coeff() == 0);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[5].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == 2);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 0);
                break;
                
                case 3:
                CHECK(t.coeff() == Fraction(1, 5));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[6].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == Fraction(5, 3));
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 0);
                break;
                
                case 3:
                CHECK(t.coeff() == Fraction(1, 3));
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = solver[7].invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == 112);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                CHECK(t.coeff() == 0);
                break;
                
                case 2:
                CHECK(t.coeff() == 7);
                break;
                
                case 3:
                CHECK(t.coeff() == 0);
                break;
                
                default: CHECK(0 == 1);
            }
        }
        
        s = special.invert_to_dual().solve().back();
        CHECK(s.valid());
        CHECK(s.basis.size() == 3);
        CHECK(s.w == 0);
        for(auto t: s.basis) {
            switch(t.idx()) {
                case 1:
                case 2:
                case 3:
                CHECK(t.coeff() == 0);
                break;
                
                default: CHECK(0 == 1);
            }
        }
    }
}

int main(int argc, char* args[]) {
    return UnitTest::RunAllTests();
}
