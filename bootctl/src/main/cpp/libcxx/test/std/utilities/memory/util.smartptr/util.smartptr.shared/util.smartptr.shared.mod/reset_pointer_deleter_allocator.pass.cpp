//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template<class Y, class D, class A> void reset(Y* p, D d, A a);

#include <memory>
#include <cassert>
#include "deleter_types.h"
#include "test_allocator.h"

struct B
{
    static int count;

    B() {++count;}
    B(const B&) {++count;}
    virtual ~B() {--count;}
};

int B::count = 0;

struct A
    : public B
{
    static int count;

    A() {++count;}
    A(const A&) {++count;}
    ~A() {--count;}
};

int A::count = 0;

int main()
{
    {
        std::shared_ptr<B> p(new B);
        A* ptr = new A;
        p.reset(ptr, test_deleter<A>(3), test_allocator<A>(4));
        assert(A::count == 1);
        assert(B::count == 1);
        assert(p.use_count() == 1);
        assert(p.get() == ptr);
        test_deleter<A>* d = std::get_deleter<test_deleter<A> >(p);
        assert(test_deleter<A>::count == 1);
        assert(test_deleter<A>::dealloc_count == 0);
        assert(d);
        assert(d->state() == 3);
        assert(test_allocator<A>::count == 1);
        assert(test_allocator<A>::alloc_count == 1);
    }
    assert(A::count == 0);
    assert(test_deleter<A>::count == 0);
    assert(test_deleter<A>::dealloc_count == 1);
    assert(test_allocator<A>::count == 0);
    assert(test_allocator<A>::alloc_count == 0);
    {
        std::shared_ptr<B> p;
        A* ptr = new A;
        p.reset(ptr, test_deleter<A>(3), test_allocator<A>(4));
        assert(A::count == 1);
        assert(B::count == 1);
        assert(p.use_count() == 1);
        assert(p.get() == ptr);
        test_deleter<A>* d = std::get_deleter<test_deleter<A> >(p);
        assert(test_deleter<A>::count == 1);
        assert(test_deleter<A>::dealloc_count == 1);
        assert(d);
        assert(d->state() == 3);
        assert(test_allocator<A>::count == 1);
        assert(test_allocator<A>::alloc_count == 1);
    }
    assert(A::count == 0);
    assert(test_deleter<A>::count == 0);
    assert(test_deleter<A>::dealloc_count == 2);
    assert(test_allocator<A>::count == 0);
    assert(test_allocator<A>::alloc_count == 0);
}
