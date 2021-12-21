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

// value_type sum() const;

#include <valarray>
#include <cassert>

int main()
{
    {
        typedef double T;
        T a1[] = {1.5, 2.5, 3, 4, 5.5};
        const unsigned N1 = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N1);
        assert(v1.sum() == 16.5);
    }
}
