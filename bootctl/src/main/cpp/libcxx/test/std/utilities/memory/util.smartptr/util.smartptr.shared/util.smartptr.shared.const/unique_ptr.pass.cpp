//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: sanitizer-new-delete

// <memory>

// template <class Y, class D> explicit shared_ptr(unique_ptr<Y, D>&&r);

#include <memory>
#include <new>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"
#include "count_new.hpp"

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

void fn ( const std::shared_ptr<int> &) {}
void fn ( const std::shared_ptr<B> &) { assert (false); }

template <typename T>
void assert_deleter ( T * ) { assert(false); }

int main()
{
    {
        std::unique_ptr<A> ptr(new A);
        A* raw_ptr = ptr.get();
        std::shared_ptr<B> p(std::move(ptr));
        assert(A::count == 1);
        assert(B::count == 1);
        assert(p.use_count() == 1);
        assert(p.get() == raw_ptr);
        assert(ptr.get() == 0);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    assert(A::count == 0);
    {
        std::unique_ptr<A> ptr(new A);
        A* raw_ptr = ptr.get();
        globalMemCounter.throw_after = 0;
        try
        {
            std::shared_ptr<B> p(std::move(ptr));
            assert(false);
        }
        catch (...)
        {
#if TEST_STD_VER >= 11
            assert(A::count == 1);
            assert(B::count == 1);
            assert(ptr.get() == raw_ptr);
#else
            (void) raw_ptr; // silence 'unused variable' warning
            assert(A::count == 0);
            assert(B::count == 0);
            assert(ptr.get() == 0);
#endif
        }
    }
#endif
    assert(A::count == 0);
    { // LWG 2399
        fn(std::unique_ptr<int>(new int));
    }
#if TEST_STD_VER >= 14
    { // LWG 2415
        std::unique_ptr<int, void (*)(int*)> p(nullptr, assert_deleter<int>);
        std::shared_ptr<int> p2(std::move(p)); // should not call deleter when going out of scope
    }
#endif
}
