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
// REQUIRES: c++98 || c++03 || c++11 || c++14

// template<class Y> shared_ptr& operator=(auto_ptr<Y>&& r);

#include <memory>
#include <type_traits>
#include <cassert>

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
        std::auto_ptr<A> pA(new A);
        A* ptrA = pA.get();
        {
            std::shared_ptr<B> pB(new B);
            pB = std::move(pA);
            assert(B::count == 1);
            assert(A::count == 1);
            assert(pB.use_count() == 1);
            assert(pA.get() == 0);
            assert(pB.get() == ptrA);
        }
        assert(B::count == 0);
        assert(A::count == 0);
    }
    assert(B::count == 0);
    assert(A::count == 0);
    {
        std::auto_ptr<A> pA;
        A* ptrA = pA.get();
        {
            std::shared_ptr<B> pB(new B);
            pB = std::move(pA);
            assert(B::count == 0);
            assert(A::count == 0);
            assert(pB.use_count() == 1);
            assert(pA.get() == 0);
            assert(pB.get() == ptrA);
        }
        assert(B::count == 0);
        assert(A::count == 0);
    }
    assert(B::count == 0);
    assert(A::count == 0);
    {
        std::auto_ptr<A> pA(new A);
        A* ptrA = pA.get();
        {
            std::shared_ptr<B> pB;
            pB = std::move(pA);
            assert(B::count == 1);
            assert(A::count == 1);
            assert(pB.use_count() == 1);
            assert(pA.get() == 0);
            assert(pB.get() == ptrA);
        }
        assert(B::count == 0);
        assert(A::count == 0);
    }
    assert(B::count == 0);
    assert(A::count == 0);
    {
        std::auto_ptr<A> pA;
        A* ptrA = pA.get();
        {
            std::shared_ptr<B> pB;
            pB = std::move(pA);
            assert(B::count == 0);
            assert(A::count == 0);
            assert(pB.use_count() == 1);
            assert(pA.get() == 0);
            assert(pB.get() == ptrA);
        }
        assert(B::count == 0);
        assert(A::count == 0);
    }
    assert(B::count == 0);
    assert(A::count == 0);
}
