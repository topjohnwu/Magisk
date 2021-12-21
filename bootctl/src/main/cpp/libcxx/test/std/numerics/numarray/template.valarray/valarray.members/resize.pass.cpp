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

// void resize(size_t n, value_type x = value_type());

#include <valarray>
#include <cassert>
#include <cstddef>

int main()
{
    {
        typedef int T;
        T a1[] = {1, 2, 3, 4, 5};
        const unsigned N1 = sizeof(a1)/sizeof(a1[0]);
        std::valarray<T> v1(a1, N1);
        v1.resize(8);
        assert(v1.size() == 8);
        for (std::size_t i = 0; i < v1.size(); ++i)
            assert(v1[i] == 0);
        v1.resize(0);
        assert(v1.size() == 0);
        v1.resize(80);
        assert(v1.size() == 80);
        for (std::size_t i = 0; i < v1.size(); ++i)
            assert(v1[i] == 0);
        v1.resize(40);
        assert(v1.size() == 40);
        for (std::size_t i = 0; i < v1.size(); ++i)
            assert(v1[i] == 0);
    }
}
