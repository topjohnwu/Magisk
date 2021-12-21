//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// template <MoveConstructible  R, MoveConstructible ... ArgTypes>
//   void swap(function<R(ArgTypes...)>&, function<R(ArgTypes...)>&) noexcept;


#include <functional>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"
#include "count_new.hpp"

class A
{
    int data_[10];
public:
    static int count;

    explicit A(int j)
    {
        ++count;
        data_[0] = j;
    }

    A(const A& a)
    {
        ++count;
        for (int i = 0; i < 10; ++i)
            data_[i] = a.data_[i];
    }

    ~A() {--count;}

    int operator()(int i) const
    {
        for (int j = 0; j < 10; ++j)
            i += data_[j];
        return i;
    }

    int id() const {return data_[0];}
};

int A::count = 0;

int g(int) {return 0;}
int h(int) {return 1;}

int main()
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    std::function<int(int)> f1 = A(1);
    std::function<int(int)> f2 = A(2);
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "" );
#endif
    assert(A::count == 2);
    assert(globalMemCounter.checkOutstandingNewEq(2));
    assert(f1.target<A>()->id() == 1);
    assert(f2.target<A>()->id() == 2);
    swap(f1, f2);
    assert(A::count == 2);
    assert(globalMemCounter.checkOutstandingNewEq(2));
    assert(f1.target<A>()->id() == 2);
    assert(f2.target<A>()->id() == 1);
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    std::function<int(int)> f1 = A(1);
    std::function<int(int)> f2 = g;
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "" );
#endif
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f1.target<A>()->id() == 1);
    assert(*f2.target<int(*)(int)>() == g);
    swap(f1, f2);
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(*f1.target<int(*)(int)>() == g);
    assert(f2.target<A>()->id() == 1);
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    std::function<int(int)> f1 = g;
    std::function<int(int)> f2 = A(1);
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "" );
#endif
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(*f1.target<int(*)(int)>() == g);
    assert(f2.target<A>()->id() == 1);
    swap(f1, f2);
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f1.target<A>()->id() == 1);
    assert(*f2.target<int(*)(int)>() == g);
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    std::function<int(int)> f1 = g;
    std::function<int(int)> f2 = h;
#if TEST_STD_VER >= 11
    static_assert(noexcept(swap(f1, f2)), "" );
#endif
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(*f1.target<int(*)(int)>() == g);
    assert(*f2.target<int(*)(int)>() == h);
    swap(f1, f2);
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(*f1.target<int(*)(int)>() == h);
    assert(*f2.target<int(*)(int)>() == g);
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
