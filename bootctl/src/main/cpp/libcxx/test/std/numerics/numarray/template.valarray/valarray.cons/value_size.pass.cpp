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

// valarray(const value_type& x, size_t n);

#include <valarray>
#include <cassert>

int main()
{
    {
        std::valarray<int> v(5, 100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i] == 5);
    }
    {
        std::valarray<double> v(2.5, 100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i] == 2.5);
    }
    {
        std::valarray<std::valarray<double> > v(std::valarray<double>(10), 100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i].size() == 10);
    }
}
