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

// template<class RealType = double>
// class piecewise_constant_distribution

// param_type(initializer_list<result_type> bl, UnaryOperation fw);

#include <random>
#include <cassert>

double f(double x)
{
    return x*2;
}

int main()
{
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        P pa({}, f);
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
        P pa({12}, f);
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
        P pa({12, 14}, f);
        std::vector<double> iv = pa.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 12);
        assert(iv[1] == 14);
        std::vector<double> dn = pa.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 0.5);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        P pa({5.5, 7.5, 11.5}, f);
        std::vector<double> iv = pa.intervals();
        assert(iv.size() == 3);
        assert(iv[0] == 5.5);
        assert(iv[1] == 7.5);
        assert(iv[2] == 11.5);
        std::vector<double> dn = pa.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 0.203125);
        assert(dn[1] == 0.1484375);
    }
}
