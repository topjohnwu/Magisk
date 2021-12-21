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

// template<class T>
//   valarray<bool>
//   operator>(const valarray<T>& x, const valarray<T>& y);

#include <valarray>
#include <cassert>
#include <cstddef>

int main()
{
    {
        typedef int T;
        T a1[] = {1,  2,  0,  4, 10};
        T a2[] = {6,  7,  0,  2, 1};
        bool a3[] = {false,  false,  false,  true,  true};
        const unsigned N = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N);
        std::valarray<T> v2(a2, N);
        std::valarray<bool> v3 = v1 > v2;
        assert(v1.size() == v2.size());
        assert(v1.size() == v3.size());
        for (std::size_t i = 0; i < v1.size(); ++i)
            assert(v3[i] == a3[i]);
    }
}
