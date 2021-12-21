//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template<class T> class valarray;

// valarray(const value_type* p, size_t n);

#include <valarray>
#include <cassert>
#include <cstddef>

int main()
{
    {
        typedef int T;
        T a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v(a, N);
        assert(v.size() == N);
        for (unsigned i = 0; i < N; ++i)
            assert(v[i] == a[i]);
    }
    {
        typedef double T;
        T a[] = {1, 2.5, 3, 4.25, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v(a, N);
        assert(v.size() == N);
        for (unsigned i = 0; i < N; ++i)
            assert(v[i] == a[i]);
    }
    {
        typedef std::valarray<double> T;
        T a[] = {T(1), T(2), T(3), T(4), T(5)};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v(a, N);
        assert(v.size() == N);
        for (unsigned i = 0; i < N; ++i)
        {
            assert(v[i].size() == a[i].size());
            for (std::size_t j = 0; j < v[i].size(); ++j)
                assert(v[i][j] == a[i][j]);
        }
    }
}
