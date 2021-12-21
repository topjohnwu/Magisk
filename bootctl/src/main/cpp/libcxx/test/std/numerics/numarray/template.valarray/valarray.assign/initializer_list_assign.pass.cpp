//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <valarray>

// template<class T> class valarray;

// valarray& operator=(initializer_list<value_type> il);

#include <valarray>
#include <cassert>
#include <cstddef>

struct S
{
    S() : x_(0) { default_ctor_called = true; }
    S(int x) : x_(x) {}
    int x_;
    static bool default_ctor_called;
};

bool S::default_ctor_called = false;

bool operator==(const S& lhs, const S& rhs)
{
    return lhs.x_ == rhs.x_;
}

int main()
{
    {
        typedef int T;
        T a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v2;
        v2 = {1, 2, 3, 4, 5};
        assert(v2.size() == N);
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == a[i]);
    }
    {
        typedef double T;
        T a[] = {1, 2.5, 3, 4.25, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v2;
        v2 = {1, 2.5, 3, 4.25, 5};
        assert(v2.size() == N);
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == a[i]);
    }
    {
        typedef std::valarray<double> T;
        T a[] = {T(1), T(2), T(3), T(4), T(5)};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v2(a, N-2);
        v2 = {T(1), T(2), T(3), T(4), T(5)};
        assert(v2.size() == N);
        for (unsigned i = 0; i < N; ++i)
        {
            assert(v2[i].size() == a[i].size());
            for (std::size_t j = 0; j < a[i].size(); ++j)
                assert(v2[i][j] == a[i][j]);
        }
    }
    {
        typedef S T;
        T a[] = {T(1), T(2), T(3), T(4), T(5)};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v2;
        v2 = {T(1), T(2), T(3), T(4), T(5)};
        assert(v2.size() == N);
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == a[i]);
        assert(!S::default_ctor_called);
    }
}
