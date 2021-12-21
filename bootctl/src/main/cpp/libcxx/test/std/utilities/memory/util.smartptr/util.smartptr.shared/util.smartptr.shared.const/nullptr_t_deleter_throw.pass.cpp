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

// shared_ptr

// template<class D> shared_ptr(nullptr_t, D d);

#include <memory>
#include <cassert>
#include <new>
#include <cstdlib>

#include "test_macros.h"
#include "count_new.hpp"

#include "deleter_types.h"

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
    globalMemCounter.throw_after = 0;
    try
    {
        std::shared_ptr<A> p(nullptr, test_deleter<A>(3));
        assert(false);
    }
    catch (std::bad_alloc&)
    {
        assert(A::count == 0);
        assert(test_deleter<A>::count == 0);
        assert(test_deleter<A>::dealloc_count == 1);
    }
}
