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

// template<class InputIterator>
//     piecewise_constant_distribution(InputIteratorB firstB,
//                                     InputIteratorB lastB,
//                                     InputIteratorW firstW);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10};
        double p[] = {12};
        D d(b, b, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 1);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10};
        double p[] = {12};
        D d(b, b+1, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 0);
        assert(iv[1] == 1);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 1);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10, 15};
        double p[] = {12};
        D d(b, b+2, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 10);
        assert(iv[1] == 15);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 1);
        assert(dn[0] == 1/5.);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10, 15, 16};
        double p[] = {.25, .75};
        D d(b, b+3, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 3);
        assert(iv[0] == 10);
        assert(iv[1] == 15);
        assert(iv[2] == 16);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == .25/5.);
        assert(dn[1] == .75);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10, 14, 16, 17};
        double p[] = {25, 62.5, 12.5};
        D d(b, b+4, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 4);
        assert(iv[0] == 10);
        assert(iv[1] == 14);
        assert(iv[2] == 16);
        assert(iv[3] == 17);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 3);
        assert(dn[0] == .0625);
        assert(dn[1] == .3125);
        assert(dn[2] == .125);
    }
}
