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
// class piecewise_linear_distribution

// piecewise_linear_distribution(initializer_list<result_type> bl,
//                                 UnaryOperation fw);

#include <iostream>

#include <random>
#include <cassert>

double f(double x)
{
    return x*2;
}

int main()
{
    {
        typedef std::piecewise_linear_distribution<> D;
        D d({}, f);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 1);
        assert(dn[1] == 1);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        D d({12}, f);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 1);
        assert(dn[1] == 1);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        D d({10, 12}, f);
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
        D d({6, 10, 14}, f);
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
