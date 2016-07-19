#include "BalanceMatrix.h"

#include <UnitTest++/UnitTest++.h>
#include <iostream>

using std::vector;

SUITE(BalanceMatrix) {
    TEST(Initialization) {
        BalanceMatrix m;
        CHECK(m.set({
            {5, 8, 4, 4, 80},
            {1, 2, 3, 8, 45},
            {4, 7, 6, 1, 60},
            {45, 60, 70, 40}
        }));
        
        std::stringstream ss;
        ss << m;
        CHECK(ss.str() == "[Balance:\n"
                          "   5   8   4   4|  80\n"
                          "   1   2   3   8|  45\n"
                          "   4   7   6   1|  60\n"
                          "  45  60  70  40\n"
                          "]"
        );
        
        CHECK(m.set({
            {5, 8, 4, 4, 80},
            {1, 2, 3, 8},
        }));
        
        ss.str(""); ss << m;
        CHECK(ss.str() == "[Balance:\n"
                          "   5   8   4   4|  80\n"
                          "   1   2   3   8\n"
                          "]"
        );
        
        CHECK(!m.set({
            {5, 8, 4, 4, 80}
        }));
        CHECK(!m.set({}));
        
        ss.str(""); ss << m;
        CHECK(ss.str() == "[Balance:\n"
                          "   5   8   4   4|  80\n"
                          "   1   2   3   8\n"
                          "]"
        );
    }
    
    struct MatricesFixture {
        MatricesFixture() {
            m[0].set({
                { 5,  8,  4,  4, 80},
                { 1,  2,  3,  8, 45},
                { 4,  7,  6,  1, 60},
                {45, 60, 70, 40}
            });
            
            m[1].set({
                { 4,  5,  5,  7, 10},
                { 8,  7,  5,  4, 20},
                { 1,  6,  4,  5, 50},
                { 3,  2,  1,  3, 30},
                {40, 30, 20, 40}
            });
            
            m[2].set({
                { 3,  1,  4,  7, 30},
                { 7,  3,  5,  8, 85},
                { 6,  3,  4,  6, 45},
                {40, 35, 15, 60}
            });
            
            m[3].set({
                { 4,  2,  4,  1, 50},
                { 2,  3,  6,  5, 30},
                { 6,  2,  4,  1, 20},
                {30, 30, 10, 20}
            });
            
            m[4].set({
                {7, 8, 5, 3, 11},
                {2, 4, 5, 9, 11},
                {6, 3, 1, 2,  8},
                {5, 9, 9, 7}
            });
        }
        
        BalanceMatrix m[5];
    };
    
    TEST_FIXTURE(MatricesFixture, NWMethod) {
        auto key = m[0].get_key0_by_nw_method();
        CHECK((key == vector<vector<int>> {
            {45, 35,  0,  0},
            { 0, 25, 20,  0},
            { 0,  0, 50, 10},
            { 0,  0,  0, 30}
        }));
        
        key = m[1].get_key0_by_nw_method();
        CHECK((key == vector<vector<int>> {
            {10,  0,  0,  0},
            {20,  0,  0,  0},
            {10, 30, 10,  0},
            { 0,  0, 10, 20},
            { 0,  0,  0, 20}
        }));
        
        key = m[2].get_key0_by_nw_method();
        CHECK((key == vector<vector<int>> {
            {30,  0,  0,  0,  0},
            {10, 35, 15, 25,  0},
            { 0,  0,  0, 35, 10}
        }));
        
        key = m[3].get_key0_by_nw_method();
        CHECK((key == vector<vector<int>> {
            {30, 20,  0,  0,  0},
            { 0, 10, 10, 10,  0},
            { 0,  0,  0, 10, 10}
        }));
        
        key = m[4].get_key0_by_nw_method();
        CHECK((key == vector<vector<int>> {
            {5, 6, 0, 0},
            {0, 3, 8, 0},
            {0, 0, 1, 7}
        }));
    }
    
    TEST_FIXTURE(MatricesFixture, MinMethod) {
        auto key = m[0].get_key0_by_min_method();
        CHECK((key == vector<vector<int>> {
            { 0, 10, 70,  0},
            {45,  0,  0,  0},
            { 0, 20,  0, 40},
            { 0, 30,  0,  0}
        }));
        
        key = m[1].get_key0_by_min_method();
        CHECK((key == vector<vector<int>> {
            { 0, 10,  0,  0},
            { 0,  0,  0, 20},
            {40,  0,  0, 10},
            { 0, 10, 20,  0},
            { 0, 10,  0, 10}
        }));
        
        key = m[2].get_key0_by_min_method();
        CHECK((key == vector<vector<int>> {
            { 0, 30,  0,  0,  0},
            {10,  5,  0, 60, 10},
            {30,  0, 15,  0,  0}
        }));
        
        key = m[3].get_key0_by_min_method();
        CHECK((key == vector<vector<int>> {
            { 0, 30,  0, 20,  0},
            {30,  0,  0,  0,  0},
            { 0,  0, 10,  0, 10}
        }));
        
        key = m[4].get_key0_by_min_method();
        CHECK((key == vector<vector<int>> {
            {0, 3, 1, 7},
            {5, 6, 0, 0},
            {0, 0, 8, 0}
        }));
    }
    
    TEST_FIXTURE(MatricesFixture, PotentialMethod) {
        auto lastStepNW = m[0].solve(BalanceMatrix::Meth::NW).back();
        auto lastStepMin = m[0].solve(BalanceMatrix::Meth::Min).back();
        CHECK(lastStepNW.valid());
        CHECK((lastStepNW.X == vector<vector<int>> {
            {10,  0, 70,  0},
            {15, 30,  0,  0},
            {20,  0,  0, 40},
            { 0, 30,  0,  0}
        }));
        CHECK((lastStepNW.D == vector<vector<int>> {
            {0, 2, 0,  2},
            {0, 0, 3, 10},
            {0, 2, 3,  0},
            {1, 0, 2,  4}
        }));
        CHECK(lastStepNW.X == lastStepMin.X);
        CHECK(lastStepNW.D == lastStepMin.D);
        CHECK(lastStepNW.W == lastStepMin.W);
        
        lastStepNW = m[1].solve(BalanceMatrix::Meth::NW).back();
        lastStepMin = m[1].solve(BalanceMatrix::Meth::Min).back();
        CHECK(lastStepNW.valid());
        CHECK((lastStepNW.X == vector<vector<int>> {
            { 0, 10,  0,  0},
            { 0,  0,  0, 20},
            {40,  0, 10, -1},
            { 0, 20, 10,  0},
            { 0,  0,  0, 20}
        }));
        CHECK((lastStepNW.D == vector<vector<int>> {
            {3, 0, 1, 2},
            {8, 3, 2, 0},
            {0, 1, 0, 0},
            {5, 0, 0, 1},
            {4, 0, 1, 0}
        }));
        CHECK((lastStepMin.X == vector<vector<int>> {
            { 0, 10,  0,  0},
            { 0,  0,  0, 20},
            {40,  0,  0, 10},
            { 0, 10, 20,  0},
            { 0, 10,  0, 10}
        }));
        CHECK(lastStepNW.D == lastStepMin.D);
        CHECK(lastStepNW.W == lastStepMin.W);
        
        lastStepNW = m[2].solve(BalanceMatrix::Meth::NW).back();
        lastStepMin = m[2].solve(BalanceMatrix::Meth::Min).back();
        CHECK((lastStepNW.X == vector<vector<int>> {
            {30,  0,  0,  0, 0}, // 30  0  0  0 0
            {10, 35, 15, 15,10}, // 10 35 15 20 5
            { 0,  0,  0, 45, 0}  //  0  0  0 40 0
        }));
        CHECK((lastStepNW.D == vector<vector<int>> {
            {0, 2, 3, 3, 4},
            {0, 0, 0, 0, 0},
            {1, 2, 1, 0, 2}
        }));
        CHECK(lastStepNW.X == lastStepMin.X);
        CHECK(lastStepNW.D == lastStepMin.D);
        CHECK(lastStepNW.W == lastStepMin.W);
        
        lastStepNW = m[3].solve(BalanceMatrix::Meth::NW).back();
        lastStepMin = m[3].solve(BalanceMatrix::Meth::Min).back();
        CHECK((lastStepNW.X == vector<vector<int>> {
            { 0, 30, 10, 10,  0},
            {30,  0,  0,  0, -1},
            { 0,  0,  0, 10, 10}
        }));
        CHECK((lastStepNW.D == vector<vector<int>> {
            {2, 0, 0, 0, 0},
            {0, 1, 2, 4, 0},
            {4, 0, 0, 0, 0}
        }));
        CHECK((lastStepMin.X == vector<vector<int>> {
            {-1, 30,  0, 20,  0},
            {30,  0,  0,  0,  0},
            { 0, -2, 10,  0, 10}
        }));
        CHECK((lastStepMin.D == vector<vector<int>> {
            {0, 0, 0, 0, 0},
            {0, 3, 4, 6, 2},
            {2, 0, 0, 0, 0}
        }));
        CHECK(lastStepNW.W == lastStepMin.W);
        
        lastStepNW = m[4].solve(BalanceMatrix::Meth::NW).back();
        lastStepMin = m[4].solve(BalanceMatrix::Meth::Min).back();
        CHECK(lastStepNW.valid());
        CHECK((lastStepNW.X == vector<vector<int>> {
            {0, 0, 4, 7},
            {5, 6, 0, 0},
            {0, 3, 5, 0}
        }));
        CHECK((lastStepNW.D == vector<vector<int>> {
            {2, 1, 0, 0},
            {0, 0, 3, 9},
            {5, 0, 0, 3}
        }));
        CHECK(lastStepNW.X == lastStepMin.X);
    }
}

int main(int, char*[]) {
    return UnitTest::RunAllTests();
}
