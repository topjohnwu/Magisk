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
// class discrete_distribution

// template<class _URNG> result_type operator()(_URNG& g);

#include <random>
#include <vector>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        D d;
        const int N = 100;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            assert((double)u[i]/N == prob[i]);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {.3};
        D d(p0, p0+1);
        const int N = 100;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            assert((double)u[i]/N == prob[i]);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {.75, .25};
        D d(p0, p0+2);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {0, 1};
        D d(p0, p0+2);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        assert((double)u[0]/N == prob[0]);
        assert((double)u[1]/N == prob[1]);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {1, 0};
        D d(p0, p0+2);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        assert((double)u[0]/N == prob[0]);
        assert((double)u[1]/N == prob[1]);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {.3, .1, .6};
        D d(p0, p0+3);
        const int N = 10000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {0, 25, 75};
        D d(p0, p0+3);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {25, 0, 75};
        D d(p0, p0+3);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {25, 75, 0};
        D d(p0, p0+3);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {0, 0, 1};
        D d(p0, p0+3);
        const int N = 100;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {0, 1, 0};
        D d(p0, p0+3);
        const int N = 100;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {1, 0, 0};
        D d(p0, p0+3);
        const int N = 100;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef std::minstd_rand G;
        G g;
        double p0[] = {33, 0, 0, 67};
        D d(p0, p0+3);
        const int N = 1000000;
        std::vector<D::result_type> u(d.max()+1);
        for (int i = 0; i < N; ++i)
        {
            D::result_type v = d(g);
            assert(d.min() <= v && v <= d.max());
            u[v]++;
        }
        std::vector<double> prob = d.probabilities();
        for (int i = 0; i <= d.max(); ++i)
            if (prob[i] != 0)
                assert(std::abs((double)u[i]/N - prob[i]) / prob[i] < 0.001);
            else
                assert(u[i] == 0);
    }
}
