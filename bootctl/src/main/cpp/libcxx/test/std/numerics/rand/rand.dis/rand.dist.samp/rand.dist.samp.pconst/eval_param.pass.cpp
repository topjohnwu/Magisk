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
// class piecewise_constant_distribution

// template<class _URNG> result_type operator()(_URNG& g, const param_type& parm);

#include <random>
#include <algorithm>
#include <vector>
#include <iterator>
#include <numeric>
#include <cassert>
#include <cstddef>

template <class T>
inline
T
sqr(T x)
{
    return x*x;
}

int main()
{
    {
        typedef std::piecewise_constant_distribution<> D;
        typedef D::param_type P;
        typedef std::mt19937_64 G;
        G g;
        double b[] = {10, 14, 16, 17};
        double p[] = {25, 62.5, 12.5};
        const size_t Np = sizeof(p) / sizeof(p[0]);
        D d;
        P pa(b, b+Np+1, p);
        const int N = 1000000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, pa);
            assert(10 <= v && v < 17);
            u.push_back(v);
        }
        std::vector<double> prob(std::begin(p), std::end(p));
        double s = std::accumulate(prob.begin(), prob.end(), 0.0);
        for (std::size_t i = 0; i < prob.size(); ++i)
            prob[i] /= s;
        std::sort(u.begin(), u.end());
        for (std::size_t i = 0; i < Np; ++i)
        {
            typedef std::vector<D::result_type>::iterator I;
            I lb = std::lower_bound(u.begin(), u.end(), b[i]);
            I ub = std::lower_bound(u.begin(), u.end(), b[i+1]);
            const size_t Ni = ub - lb;
            if (prob[i] == 0)
                assert(Ni == 0);
            else
            {
                assert(std::abs((double)Ni/N - prob[i]) / prob[i] < .01);
                double mean = std::accumulate(lb, ub, 0.0) / Ni;
                double var = 0;
                double skew = 0;
                double kurtosis = 0;
                for (I j = lb; j != ub; ++j)
                {
                    double dbl = (*j - mean);
                    double d2 = sqr(dbl);
                    var += d2;
                    skew += dbl * d2;
                    kurtosis += d2 * d2;
                }
                var /= Ni;
                double dev = std::sqrt(var);
                skew /= Ni * dev * var;
                kurtosis /= Ni * var * var;
                kurtosis -= 3;
                double x_mean = (b[i+1] + b[i]) / 2;
                double x_var = sqr(b[i+1] - b[i]) / 12;
                double x_skew = 0;
                double x_kurtosis = -6./5;
                assert(std::abs((mean - x_mean) / x_mean) < 0.01);
                assert(std::abs((var - x_var) / x_var) < 0.01);
                assert(std::abs(skew - x_skew) < 0.01);
                assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
            }
        }
    }
}
