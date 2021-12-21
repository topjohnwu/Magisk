//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType = double>
// class piecewise_linear_distribution

// template<class UnaryOperation>
//     piecewise_linear_distribution(size_t nw, result_type xmin,
//                                     result_type xmax, UnaryOperation fw);

#include <iostream>

#include <random>
#include <cassert>

double fw(double x)
{
    return 2*x;
}

int main()
{
    {
        typedef std::piecewise_linear_distribution<> D;
        D d(0, 0, 1, fw);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 0);
        assert(dn[1] == 2);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        D d(1, 10, 12, fw);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 10);
        assert(iv[1] == 12);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 20./44);
        assert(dn[1] == 24./44);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        D d(2, 6, 14, fw);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 3);
        assert(iv[0] == 6);
        assert(iv[1] == 10);
        assert(iv[2] == 14);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 3);
        assert(dn[0] == 0.075);
        assert(dn[1] == 0.125);
        assert(dn[2] == 0.175);
    }
}
