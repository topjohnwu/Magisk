//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class _IntType = int>
// class uniform_int_distribution

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
        typedef std::uniform_int_distribution<> D;
        typedef std::minstd_rand G;
        typedef D::param_type P;
        G g;
        D d(5, 100);
        P p(-10, 20);
        const int N = 100000;
        std::vector<D::result_type> u;
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g, p);
            assert(p.a() <= v && v <= p.b());
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
        double x_mean = ((double)p.a() + p.b()) / 2;
        double x_var = (sqr((double)p.b() - p.a() + 1) - 1) / 12;
        double x_skew = 0;
        double x_kurtosis = -6. * (sqr((double)p.b() - p.a() + 1) + 1) /
                            (5. * (sqr((double)p.b() - p.a() + 1) - 1));
        assert(std::abs((mean - x_mean) / x_mean) < 0.01);
        assert(std::abs((var - x_var) / x_var) < 0.01);
        assert(std::abs(skew - x_skew) < 0.01);
        assert(std::abs((kurtosis - x_kurtosis) / x_kurtosis) < 0.01);
    }
}
