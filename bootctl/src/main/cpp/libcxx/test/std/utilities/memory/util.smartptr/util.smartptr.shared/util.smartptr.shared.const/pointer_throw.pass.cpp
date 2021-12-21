//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// UNSUPPORTED: sanitizer-new-delete

// <memory>

// template<class Y> explicit shared_ptr(Y* p);


#include <memory>
#include <new>
#include <cstdlib>
#include <cassert>

#include "count_new.hpp"

struct A
{
    static int count;

    A() {++count;}
    A(const A&) {++count;}
    ~A() {--count;}
};

int A::count = 0;


int main()
{
    A* ptr = new A;
    assert(A::count == 1);
    globalMemCounter.throw_after = 0;
    try
    {
        std::shared_ptr<A> p(ptr);
        assert(false);
    }
    catch (std::bad_alloc&)
    {
        assert(A::count == 0);
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
