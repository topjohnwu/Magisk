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

// template <class T>
//   unspecified1
//   end(valarray<T>& v);

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
        *(end(v) - 1) = 10;
        assert(v[v.size()-1] == 10);
        assert(static_cast<std::size_t>(end(v) - begin(v)) == v.size());
    }
}
