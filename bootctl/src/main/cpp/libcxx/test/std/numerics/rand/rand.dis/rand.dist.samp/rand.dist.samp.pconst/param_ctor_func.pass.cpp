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
// class piecewise_constant_distribution

// template<class UnaryOperation>
//     param_type(size_t nw, double xmin, double xmax,
//                           UnaryOperation fw);

#include <random>
#include <cassert>

double fw(double x)
{
    return 2*x;
}

int main()
{
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        P pa(0, 0, 1, fw);
        std::vector<double> iv = pa.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = pa.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 1);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        P pa(1, 10, 12, fw);
        std::vector<double> iv = pa.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 10);
        assert(iv[1] == 12);
        std::vector<double> dn = pa.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 0.5);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        P pa(2, 6, 14, fw);
        std::vector<double> iv = pa.intervals();
        assert(iv.size() == 3);
        assert(iv[0] == 6);
        assert(iv[1] == 10);
        assert(iv[2] == 14);
        std::vector<double> dn = pa.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 0.1);
        assert(dn[1] == 0.15);
    }
}
