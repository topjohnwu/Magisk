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
// class uniform_real_distribution

// template<class _URNG> result_type operator()(_URNG& g);

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
        typedef std::uniform_real_distribution<> D;
        typedef std::minstd_rand0 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::mt19937 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::mt19937_64 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::ranlux24_base G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.02);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::ranlux48_base G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::ranlux24 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::ranlux48 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::knuth_b G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d(-1, 1);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs(mean - x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_real_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d(5.5, 25);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v < d.b());
            u.push_back(v);
        }
        D::result_type mean = std::accumulate(u.begin(), u.end(),
                                              D::result_type(0)) / u.size();
        D::result_type var = 0;
        D::result_type skew = 0;
        D::result_type kurtosis = 0;
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            D::result_type dbl = (u[i] - mean);
            D::result_type d2 = sqr(dbl);
            var += d2;
            skew += dbl * d2;
            kurtosis += d2 * d2;
        }
        var /= u.size();
        D::result_type dev = std::sqrt(var);
        skew /= u.size() * dev * var;
        kurtosis /= u.size() * var * var;
        kurtosis -= 3;
        D::result_type x_mean = (d.a() + d.b()) / 2;
        D::result_type x_var = sqr(d.b() - d.a()) / 12;
        D::result_type x_skew = 0;
        D::result_type x_kurtosis = -6./5;
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
}
