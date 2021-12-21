//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class IntType = int>
// class discrete_distribution

// template<class UnaryOperation>
//     param_type(size_t nw, double xmin, double xmax,
//                           UnaryOperation fw);

#include <random>
#include <cassert>

double fw(double x)
{
    return x+1;
}

int main()
{
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa(0, 0, 1, fw);
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa(1, 0, 1, fw);
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa(2, 0.5, 1.5, fw);
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 2);
        assert(p[0] == .4375);
        assert(p[1] == .5625);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        P pa(4, 0, 2, fw);
        std::vector<double> p = pa.probabilities();
        assert(p.size() == 4);
        assert(p[0] == .15625);
        assert(p[1] == .21875);
        assert(p[2] == .28125);
    }
}
