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

// valarray();

#include <valarray>
#include <cassert>

struct S {
    S() { ctor_called = true; }
    static bool ctor_called;
};

bool S::ctor_called = false;

int main()
{
    {
        std::valarray<int> v;
        assert(v.size() == 0);
    }
    {
        std::valarray<float> v;
        assert(v.size() == 0);
    }
    {
        std::valarray<double> v;
        assert(v.size() == 0);
    }
    {
        std::valarray<std::valarray<double> > v;
        assert(v.size() == 0);
    }
    {
        std::valarray<S> v;
        assert(v.size() == 0);
        assert(!S::ctor_called);
    }
}
