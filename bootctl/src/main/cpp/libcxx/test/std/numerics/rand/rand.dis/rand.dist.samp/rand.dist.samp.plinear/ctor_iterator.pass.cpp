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

// template<class InputIterator>
//     piecewise_linear_distribution(InputIteratorB firstB,
//                                     InputIteratorB lastB,
//                                     InputIteratorW firstW);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::piecewise_linear_distribution<> D;
        double b[] = {10};
        double p[] = {12};
        D d(b, b, p);
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
        double b[] = {10};
        double p[] = {12};
        D d(b, b+1, p);
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
        double b[] = {10, 15};
        double p[] = {20, 20};
        D d(b, b+2, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 2);
        assert(iv[0] == 10);
        assert(iv[1] == 15);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 2);
        assert(dn[0] == 1/5.);
        assert(dn[1] == 1/5.);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        double b[] = {10, 15, 16};
        double p[] = {.25, .75, .25};
        D d(b, b+3, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 3);
        assert(iv[0] == 10);
        assert(iv[1] == 15);
        assert(iv[2] == 16);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 3);
        assert(dn[0] == .25/3);
        assert(dn[1] == .75/3);
        assert(dn[2] == .25/3);
    }
    {
        typedef std::piecewise_linear_distribution<> D;
        double b[] = {10, 14, 16, 17};
        double p[] = {0, 1, 1, 0};
        D d(b, b+4, p);
        std::vector<double> iv = d.intervals();
        assert(iv.size() == 4);
        assert(iv[0] == 10);
        assert(iv[1] == 14);
        assert(iv[2] == 16);
        assert(iv[3] == 17);
        std::vector<double> dn = d.densities();
        assert(dn.size() == 4);
        assert(dn[0] == 0);
        assert(dn[1] == 1/4.5);
        assert(dn[2] == 1/4.5);
        assert(dn[3] == 0);
    }
}
