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

// template<class T> valarray<T> operator^(const T& x, const valarray<T>& y);

#include <valarray>
#include <cassert>
#include <cstddef>

int main()
{
    {
        typedef int T;
        T a1[] = { 1,   2,  3,  4,  5};
        T a2[] = { 2,   1,  0,  7,  6};
        const unsigned N = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N);
        std::valarray<T> v2 = 3 ^ v1;
        assert(v1.size() == v2.size());
        for (std::size_t i = 0; i < v2.size(); ++i)
            assert(v2[i] == a2[i]);
    }
}
