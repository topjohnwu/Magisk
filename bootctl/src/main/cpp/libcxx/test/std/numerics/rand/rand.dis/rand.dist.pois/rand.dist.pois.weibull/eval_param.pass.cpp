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
// class weibull_distribution

// template<class _URNG> result_type operator()(_URNG& g, const param_type& parm);

#include <random>
#include <cassert>
#include <vector>
#include <numeric>
#include <cstddef>

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
        typedef std::weibull_distribution<> D;
        typedef D::param_type P;
        typedef std::mt19937 G;
        G g;
        D d(0.5, 2);
        P p(1, .5);
        const int N = 1000000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v);
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
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
        double x_mean = p.b() * std::tgamma(1 + 1/p.a());
        double x_var = sqr(p.b()) * std::tgamma(1 + 2/p.a()) - sqr(x_mean);
        double x_skew = (sqr(p.b())*p.b() * std::tgamma(1 + 3/p.a()) -
                        3*x_mean*x_var - sqr(x_mean)*x_mean) /
                        (std::sqrt(x_var)*x_var);
        double x_kurtosis = (sqr(sqr(p.b())) * std::tgamma(1 + 4/p.a()) -
                       4*x_skew*x_var*sqrt(x_var)*x_mean -
                       6*sqr(x_mean)*x_var - sqr(sqr(x_mean))) / sqr(x_var) - 3;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::weibull_distribution<> D;
        typedef D::param_type P;
        typedef std::mt19937 G;
        G g;
        D d(1, .5);
        P p(2, 3);
        const int N = 1000000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v);
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
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
        double x_mean = p.b() * std::tgamma(1 + 1/p.a());
        double x_var = sqr(p.b()) * std::tgamma(1 + 2/p.a()) - sqr(x_mean);
        double x_skew = (sqr(p.b())*p.b() * std::tgamma(1 + 3/p.a()) -
                        3*x_mean*x_var - sqr(x_mean)*x_mean) /
                        (std::sqrt(x_var)*x_var);
        double x_kurtosis = (sqr(sqr(p.b())) * std::tgamma(1 + 4/p.a()) -
                       4*x_skew*x_var*sqrt(x_var)*x_mean -
                       6*sqr(x_mean)*x_var - sqr(sqr(x_mean))) / sqr(x_var) - 3;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.03);
    }
    {
        typedef std::weibull_distribution<> D;
        typedef D::param_type P;
        typedef std::mt19937 G;
        G g;
        D d(2, 3);
        P p(.5, 2);
        const int N = 1000000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(d.min() <= v);
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(), 0.0) / u.size();
        double var = 0;
        double skew = 0;
        double kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
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
        double x_mean = p.b() * std::tgamma(1 + 1/p.a());
        double x_var = sqr(p.b()) * std::tgamma(1 + 2/p.a()) - sqr(x_mean);
        double x_skew = (sqr(p.b())*p.b() * std::tgamma(1 + 3/p.a()) -
                        3*x_mean*x_var - sqr(x_mean)*x_mean) /
                        (std::sqrt(x_var)*x_var);
        double x_kurtosis = (sqr(sqr(p.b())) * std::tgamma(1 + 4/p.a()) -
                       4*x_skew*x_var*sqrt(x_var)*x_mean -
                       6*sqr(x_mean)*x_var - sqr(sqr(x_mean))) / sqr(x_var) - 3;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs((skew - x_skew) / x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.03);
    }
}
