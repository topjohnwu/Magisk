//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// REQUIRES: long_tests

// <random>

// template<class RealType = double>
// class fisher_f_distribution

// template<class _URNG> result_type operator()(_URNG& g);

#include <random>
#include <cassert>
#include <vector>
#include <algorithm>
#include <cmath>

double fac(double x)
{
    double r = 1;
    for (; x > 1; --x)
        r *= x;
    return r;
}

double
I(double x, unsigned a, unsigned b)
{
    double r = 0;
    for (int j = a; static_cast<unsigned>(j) <= a+b-1; ++j)
        r += fac(a+b-1)/(fac(j) * fac(a + b - 1 - j)) * std::pow(x, j) *
             std::pow(1-x, a+b-1-j);
    return r;
}

double
f(double x, double m, double n)
{
    return I(m * x / (m*x + n), static_cast<unsigned>(m/2), static_cast<unsigned>(n/2));
}

int main()
{
    // Purposefully only testing even integral values of m and n (for now)
    {
        typedef std::fisher_f_distribution<> D;
        typedef std::mt19937 G;
        G g;
        D d(2, 4);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(v >= 0);
            u.push_back(v);
        }
        std::sort(u.begin(), u.end());
        for (int i = 0; i < N; ++i)
            assert(std::abs(f(u[i], d.m(), d.n()) - double(i)/N) < .01);
    }
    {
        typedef std::fisher_f_distribution<> D;
        typedef std::mt19937 G;
        G g;
        D d(4, 2);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(v >= 0);
            u.push_back(v);
        }
        std::sort(u.begin(), u.end());
        for (int i = 0; i < N; ++i)
            assert(std::abs(f(u[i], d.m(), d.n()) - double(i)/N) < .01);
    }
    {
        typedef std::fisher_f_distribution<> D;
        typedef std::mt19937 G;
        G g;
        D d(18, 20);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(v >= 0);
            u.push_back(v);
        }
        std::sort(u.begin(), u.end());
        for (int i = 0; i < N; ++i)
            assert(std::abs(f(u[i], d.m(), d.n()) - double(i)/N) < .01);
    }
}
