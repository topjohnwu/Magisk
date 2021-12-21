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

// template<class _URNG> result_type operator()(_URNG& g);

#include <random>
#include <vector>
#include <iterator>
#include <numeric>
#include <algorithm>   // for sort
#include <cassert>

template <class T>
inline
T
sqr(T x)
{
    return x*x;
}

void
test1()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {25, 62.5, 12.5};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test2()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {0, 62.5, 12.5};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test3()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {25, 0, 12.5};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test4()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {25, 62.5, 0};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test5()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {25, 0, 0};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test6()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {0, 25, 0};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test7()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16, 17};
    double p[] = {0, 0, 1};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test8()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16};
    double p[] = {75, 25};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test9()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16};
    double p[] = {0, 25};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test10()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14, 16};
    double p[] = {1, 0};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

void
test11()
{
    typedef std::piecewise_constant_distribution<> D;
    typedef std::mt19937_64 G;
    G g;
    double b[] = {10, 14};
    double p[] = {1};
    const size_t Np = sizeof(p) / sizeof(p[0]);
    D d(b, b+Np+1, p);
    const int N = 100000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
        assert(d.min() <= v && v < d.max());
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

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
}
