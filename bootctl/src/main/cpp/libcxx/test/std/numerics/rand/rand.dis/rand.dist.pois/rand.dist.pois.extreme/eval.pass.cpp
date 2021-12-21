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
// class extreme_value_distribution

// template<class _URNG> result_type operator()(_URNG& g);

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

void
test1()
{
    typedef std::extreme_value_distribution<> D;
    typedef std::mt19937 G;
    G g;
    D d(0.5, 2);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
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
    double x_mean = d.a() + d.b() * 0.577215665;
    double x_var = sqr(d.b()) * 1.644934067;
    double x_skew = 1.139547;
    double x_kurtosis = 12./5;
    assert(std::abs((mean - x_mean) / x_mean) < 0.01);
    assert(std::abs((var - x_var) / x_var) < 0.01);
    assert(std::abs((skew - x_skew) / x_skew) < 0.01);
    assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
}

void
test2()
{
    typedef std::extreme_value_distribution<> D;
    typedef std::mt19937 G;
    G g;
    D d(1, 2);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
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
    double x_mean = d.a() + d.b() * 0.577215665;
    double x_var = sqr(d.b()) * 1.644934067;
    double x_skew = 1.139547;
    double x_kurtosis = 12./5;
    assert(std::abs((mean - x_mean) / x_mean) < 0.01);
    assert(std::abs((var - x_var) / x_var) < 0.01);
    assert(std::abs((skew - x_skew) / x_skew) < 0.01);
    assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
}

void
test3()
{
    typedef std::extreme_value_distribution<> D;
    typedef std::mt19937 G;
    G g;
    D d(1.5, 3);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
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
    double x_mean = d.a() + d.b() * 0.577215665;
    double x_var = sqr(d.b()) * 1.644934067;
    double x_skew = 1.139547;
    double x_kurtosis = 12./5;
    assert(std::abs((mean - x_mean) / x_mean) < 0.01);
    assert(std::abs((var - x_var) / x_var) < 0.01);
    assert(std::abs((skew - x_skew) / x_skew) < 0.01);
    assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
}

void
test4()
{
    typedef std::extreme_value_distribution<> D;
    typedef std::mt19937 G;
    G g;
    D d(3, 4);
    const int N = 1000000;
    std::vector<D::result_type> u;
    for (int i = 0; i < N; ++i)
    {
        D::result_type v = d(g);
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
    double x_mean = d.a() + d.b() * 0.577215665;
    double x_var = sqr(d.b()) * 1.644934067;
    double x_skew = 1.139547;
    double x_kurtosis = 12./5;
    assert(std::abs((mean - x_mean) / x_mean) < 0.01);
    assert(std::abs((var - x_var) / x_var) < 0.01);
    assert(std::abs((skew - x_skew) / x_skew) < 0.01);
    assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
}

int main()
{
    test1();
    test2();
    test3();
    test4();
}
