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

// template<class IntType = int>
// class poisson_distribution

// template<class _URNG> result_type operator()(_URNG& g, const param_type& parm);

#include <random>
#include <cassert>
#include <vector>
#include <numeric>

template <class T>
inline
T
sqr(T x)
{
    return x * x;
}

int main()
{
    {
        typedef std::poisson_distribution<> D;
        typedef D::param_type P;
        typedef std::minstd_rand G;
        G g;
        D d(.75);
        P p(2);
        const int N = 100000;
        std::vector<double> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v && v <= d.max());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (unsigned i = 0; i < u.size(); ++i)
        {
            double dbl = (u[i] - mean);
            double d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        double dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        double x_mean = p.mean();
        double x_var = p.mean();
        double x_skew = 1 / std::sqrt(x_var);
        double x_kurtosis = 1 / x_var;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.03);
    }
    {
        typedef std::poisson_distribution<> D;
        typedef D::param_type P;
        typedef std::minstd_rand G;
        G g;
        D d(2);
        P p(.75);
        const int N = 100000;
        std::vector<double> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v && v <= d.max());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (unsigned i = 0; i < u.size(); ++i)
        {
            double dbl = (u[i] - mean);
            double d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        double dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        double x_mean = p.mean();
        double x_var = p.mean();
        double x_skew = 1 / std::sqrt(x_var);
        double x_kurtosis = 1 / x_var;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.04);
    }
    {
        typedef std::poisson_distribution<> D;
        typedef D::param_type P;
        typedef std::mt19937 G;
        G g;
        D d(2);
        P p(20);
        const int N = 1000000;
        std::vector<double> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v && v <= d.max());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (unsigned i = 0; i < u.size(); ++i)
        {
            double dbl = (u[i] - mean);
            double d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        double dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        double x_mean = p.mean();
        double x_var = p.mean();
        double x_skew = 1 / std::sqrt(x_var);
        double x_kurtosis = 1 / x_var;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
}
