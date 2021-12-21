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

// function& operator=(nullptr_t);

#include <functional>
#include <cassert>

#include "count_new.hpp"

class A
{
    int data_[10];
public:
    static int count;

    A()
    {
        ++count;
        for (int i = 0; i < 10; ++i)
            data_[i] = i;
    }

    A(const A&) {++count;}

    ~A() {--count;}

    int operator()(int i) const
    {
        for (int j = 0; j < 10; ++j)
            i += data_[j];
        return i;
    }
};

int A::count = 0;

int g(int) {return 0;}

int main()
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    std::function<int(int)> f = A();
    assert(A::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f.target<A>());
    f = nullptr;
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f.target<A>() == 0);
    }
    {
    std::function<int(int)> f = g;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f.target<int(*)(int)>());
    assert(f.target<A>() == 0);
    f = nullptr;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f.target<int(*)(int)>() == 0);
    }
}
