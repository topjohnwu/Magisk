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

// explicit valarray(size_t);

#include <valarray>
#include <cassert>

struct S {
    S() : x(1) {}
    ~S() { ++cnt_dtor; }
    int x;
    static size_t cnt_dtor;
};

size_t S::cnt_dtor = 0;

int main()
{
    {
        std::valarray<int> v(100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i] == 0);
    }
    {
        std::valarray<double> v(100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i] == 0);
    }
    {
        std::valarray<std::valarray<double> > v(100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i].size() == 0);
    }
    {
        std::valarray<S> v(100);
        assert(v.size() == 100);
        for (int i = 0; i < 100; ++i)
            assert(v[i].x == 1);
    }
    assert(S::cnt_dtor == 100);
}
