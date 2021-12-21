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

// template<class _IntType = int>
// class uniform_int_distribution

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
        typedef std::uniform_int_distribution<> D;
        typedef std::minstd_rand0 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::mt19937 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::mt19937_64 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::ranlux24_base G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::ranlux48_base G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::ranlux24 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::ranlux48 G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::knuth_b G;
        G g;
        D d;
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::minstd_rand0 G;
        G g;
        D d(-6, 106);
        for (int i = 0; i < 10000; ++i)
        {
            int u = d(g);
            assert(-6 <= u && u <= 106);
        }
    }
    {
        typedef std::uniform_int_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d(5, 100);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.a() <= v && v <= d.b());
            u.push_back(v);
        }
        double mean = std::accumulate(u.begin(), u.end(),
                                              double(0)) / u.size();
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
        double x_mean = ((double)d.a() + d.b()) / 2;
        double x_var = (sqr((double)d.b() - d.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)d.b() - d.a() + 1) + 1) /
                            (5. * (sqr((double)d.b() - d.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
}
