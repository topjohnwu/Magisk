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

// valarray(initializer_list<value_type>);

#include <valarray>
#include <cassert>

int main()
{
    {
        typedef int T;
        T a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v = {1, 2, 3, 4, 5};
        assert(v.size() == N);
        for (unsigned i = 0; i < N; ++i)
            assert(v[i] == a[i]);
    }
    {
        typedef double T;
        T a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        std::valarray<T> v = {1, 2, 3, 4, 5};
        assert(v.size() == N);
        for (unsigned i = 0; i < N; ++i)
            assert(v[i] == a[i]);
    }
}
