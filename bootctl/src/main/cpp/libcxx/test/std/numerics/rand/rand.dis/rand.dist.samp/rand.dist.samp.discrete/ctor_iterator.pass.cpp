//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class IntType = int>
// class discrete_distribution

// template<class InputIterator>
//     discrete_distribution(InputIterator firstW, InputIterator lastW);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {1};
        D d(p0, p0);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {10};
        D d(p0, p0+1);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 1);
        assert(p[0] == 1);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {10, 30};
        D d(p0, p0+2);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 2);
        assert(p[0] == 0.25);
        assert(p[1] == 0.75);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {30, 10};
        D d(p0, p0+2);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 2);
        assert(p[0] == 0.75);
        assert(p[1] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {30, 0, 10};
        D d(p0, p0+3);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0.75);
        assert(p[1] == 0);
        assert(p[2] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {0, 30, 10};
        D d(p0, p0+3);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0);
        assert(p[1] == 0.75);
        assert(p[2] == 0.25);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {0, 0, 10};
        D d(p0, p0+3);
        std::vector<double> p = d.probabilities();
        assert(p.size() == 3);
        assert(p[0] == 0);
        assert(p[1] == 0);
        assert(p[2] == 1);
    }
}
