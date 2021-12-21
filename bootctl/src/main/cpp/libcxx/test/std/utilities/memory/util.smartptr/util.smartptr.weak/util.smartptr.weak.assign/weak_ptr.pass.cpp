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

// weak_ptr& operator=(const weak_ptr& r);

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
        const std::shared_ptr<A> ps(new A);
        const std::weak_ptr<A> pA(ps);
        {
            std::weak_ptr<A> pB;
            pB = pA;
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
        const std::shared_ptr<A> ps(new A);
        std::weak_ptr<A> pA(ps);
        {
            std::weak_ptr<A> pB;
            pB = std::move(pA);
            assert(B::count == 1);
            assert(A::count == 1);
            assert(pB.use_count() == 1);
        }
        assert(B::count == 1);
        assert(A::count == 1);
    }
    assert(B::count == 0);
    assert(A::count == 0);
}
