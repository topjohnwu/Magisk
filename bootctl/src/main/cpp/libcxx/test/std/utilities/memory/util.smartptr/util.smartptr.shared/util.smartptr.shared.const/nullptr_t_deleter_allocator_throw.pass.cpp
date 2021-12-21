//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// <memory>

// template<class D, class A> shared_ptr(nullptr_t, D d, A a);

#include <memory>
#include <cassert>
#include "deleter_types.h"
#include "test_allocator.h"

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
    try
    {
        test_allocator<A>::throw_after = 0;
        std::shared_ptr<A> p(nullptr, test_deleter<A>(3), test_allocator<A>(5));
        assert(false);
    }
    catch (std::bad_alloc&)
    {
        assert(A::count == 0);
        assert(test_deleter<A>::count == 0);
        assert(test_deleter<A>::dealloc_count == 1);
        assert(test_allocator<A>::count == 0);
        assert(test_allocator<A>::alloc_count == 0);
    }
}
