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

// piecewise_linear_distribution(initializer_list<double> wl);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::piecewise_linear_distribution<> D;
        D d;
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 1);
        assert(dn[1] == 1);
    }
}
