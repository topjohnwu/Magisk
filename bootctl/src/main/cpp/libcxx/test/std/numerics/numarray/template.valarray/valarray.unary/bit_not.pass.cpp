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

// valarray operator~() const;

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
        std::valarray<T> v2 = ~v;
        assert(v2.size() == v.size());
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == ~v[i]);
    }
    {
        typedef std::valarray<int> T;
        T a[] = {T(1), T(2), T(3), T(4), T(5)};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v(a, N);
        std::valarray<T> v2 = ~v;
        assert(v2.size() == v.size());
        for (unsigned i = 0; i < N; ++i)
        {
            assert(v2[i].size() == v[i].size());
            for (std::size_t j = 0; j < v[i].size(); ++j)
                assert(v2[i][j] == ~v[i][j]);
        }
    }
    {
        typedef int T;
        T a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v(a, N);
        std::valarray<T> v2 = ~(v + v);
        assert(v2.size() == v.size());
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == ~(2*v[i]));
    }
}
