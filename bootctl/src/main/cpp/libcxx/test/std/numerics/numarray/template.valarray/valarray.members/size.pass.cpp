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

// size_t size() const;

#include <valarray>
#include <cassert>

int main()
{
    {
        typedef int T;
        T a1[] = {1, 2, 3, 4, 5};
        const unsigned N1 = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N1);
        assert(v1.size() == N1);
    }
    {
        typedef int T;
        T a1[] = {1, 2, 3, 4, 5};
        const unsigned N1 = 0;
        std::valarray<T> v1(a1, N1);
        assert(v1.size() == N1);
    }
    {
        typedef int T;
        const unsigned N1 = 0;
        std::valarray<T> v1;
        assert(v1.size() == N1);
    }
}
