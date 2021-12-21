//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template<class Y> explicit shared_ptr(auto_ptr<Y>&& r);
// REQUIRES: c++98 || c++03 || c++11 || c++14


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

int main()
{
    {
        std::auto_ptr<A> ptr(new A);
        A* raw_ptr = ptr.get();
#if TEST_STD_VER >= 11
        std::shared_ptr<B> p(std::move(ptr));
#else
        std::shared_ptr<B> p(ptr);
#endif
        assert(A::count == 1);
        assert(B::count == 1);
        assert(p.use_count() == 1);
        assert(p.get() == raw_ptr);
        assert(ptr.get() == 0);
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
#if !defined(TEST_HAS_NO_EXCEPTIONS) && !defined(DISABLE_NEW_COUNT)
    {
        std::auto_ptr<A> ptr(new A);
        A* raw_ptr = ptr.get();
        globalMemCounter.throw_after = 0;
        try
        {
#if TEST_STD_VER >= 11
            std::shared_ptr<B> p(std::move(ptr));
#else
            std::shared_ptr<B> p(ptr);
#endif
            assert(false);
        }
        catch (...)
        {
#if TEST_STD_VER >= 11
            assert(A::count == 1);
            assert(B::count == 1);
            assert(ptr.get() == raw_ptr);
 #else
            // Without rvalue references, ptr got copied into
            // the shared_ptr destructor and the copy was
            // destroyed during unwinding.
            (void) raw_ptr; // silence 'unused variable' warning
            assert(A::count == 0);
            assert(B::count == 0);
#endif
        }
    }
    assert(A::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
#endif // !defined(TEST_HAS_NO_EXCEPTIONS) && !defined(DISABLE_NEW_COUNT)
}
