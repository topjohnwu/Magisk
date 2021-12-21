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
//   valarray<T>
//   abs(const valarray<T>& x);

#include <valarray>
#include <cassert>
#include <cstddef>

int main()
{
    {
        typedef double T;
        T a1[] = {1.5,  -2.5,  3.4,  -4.5,  -5.0};
        T a3[] = {1.5,   2.5,  3.4,   4.5,   5.0};
        const unsigned N = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N);
        std::valarray<T> v3 = abs(v1);
        assert(v3.size() == v1.size());
        for (std::size_t i = 0; i < v3.size(); ++i)
            assert(v3[i] == a3[i]);
    }
}
