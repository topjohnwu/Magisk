//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// template<Returnable R, class T, CopyConstructible... Args>
//   unspecified mem_fn(R (T::* pm)(Args...));

#include <functional>
#include <cassert>

#include "test_macros.h"

struct A
{
    char test0() {return 'a';}
    char test1(int) {return 'b';}
    char test2(int, double) {return 'c';}
};

template <class F>
void
test0(F f)
{
    {
    A a;
    assert(f(a) == 'a');
    A* ap = &a;
    assert(f(ap) == 'a');
    const F& cf = f;
    assert(cf(ap) == 'a');
    }
}

template <class F>
void
test1(F f)
{
    {
    A a;
    assert(f(a, 1) == 'b');
    A* ap = &a;
    assert(f(ap, 2) == 'b');
    const F& cf = f;
    assert(cf(ap, 2) == 'b');
    }
}

template <class F>
void
test2(F f)
{
    {
    A a;
    assert(f(a, 1, 2) == 'c');
    A* ap = &a;
    assert(f(ap, 2, 3.5) == 'c');
    const F& cf = f;
    assert(cf(ap, 2, 3.5) == 'c');
    }
}

int main()
{
    test0(std::mem_fn(&A::test0));
    test1(std::mem_fn(&A::test1));
    test2(std::mem_fn(&A::test2));
#if TEST_STD_VER >= 11
    static_assert((noexcept(std::mem_fn(&A::test0))), ""); // LWG#2489
#endif
}
