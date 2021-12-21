//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <random>

// template<class IntType = int>
// class discrete_distribution

// param_type(initializer_list<double> wl);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {10};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {10, 30};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 2);
        assert(p[0] == 0.25);
        assert(p[1] == 0.75);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {30, 10};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 2);
        assert(p[0] == 0.75);
        assert(p[1] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {30, 0, 10};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0.75);
        assert(p[1] == 0);
        assert(p[2] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {0, 30, 10};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0);
        assert(p[1] == 0.75);
        assert(p[2] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa = {0, 0, 10};
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0);
        assert(p[1] == 0);
        assert(p[2] == 1);
    }
}
