//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// weak_ptr

// weak_ptr(const weak_ptr& r);
// weak_ptr(weak_ptr &&r)

#include <memory>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

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

struct C
{
    static int count;

    C() {++count;}
    C(const C&) {++count;}
    virtual ~C() {--count;}
};

int C::count = 0;

template <class T>
std::weak_ptr<T> source (std::shared_ptr<T> p) { return std::weak_ptr<T>(p); }

#if TEST_STD_VER >= 11
template <class T>
void sink (std::weak_ptr<T> &&) {}
#endif

int main()
{
    {
        const std::shared_ptr<A> ps(new A);
        const std::weak_ptr<A> pA(ps);
        assert(pA.use_count() == 1);
        assert(B::count == 1);
        assert(A::count == 1);
        {
            std::weak_ptr<A> pB(pA);
            assert(B::count == 1);
            assert(A::count == 1);
            assert(pB.use_count() == 1);
            assert(pA.use_count() == 1);
        }
        assert(pA.use_count() == 1);
        assert(B::count == 1);
        assert(A::count == 1);
    }
    assert(B::count == 0);
    assert(A::count == 0);
    {
        std::weak_ptr<A> pA;
        assert(pA.use_count() == 0);
        assert(B::count == 0);
        assert(A::count == 0);
        {
            std::weak_ptr<A> pB(pA);
            assert(B::count == 0);
            assert(A::count == 0);
            assert(pB.use_count() == 0);
            assert(pA.use_count() == 0);
        }
        assert(pA.use_count() == 0);
        assert(B::count == 0);
        assert(A::count == 0);
    }
    assert(B::count == 0);
    assert(A::count == 0);

#if TEST_STD_VER >= 11
    {
        std::shared_ptr<A> ps(new A);
        std::weak_ptr<A> pA = source(ps);
        assert(pA.use_count() == 1);
        assert(A::count == 1);
        sink(std::move(pA)); // kill off the weak pointer
    }
    assert(B::count == 0);
    assert(A::count == 0);
#endif
}
