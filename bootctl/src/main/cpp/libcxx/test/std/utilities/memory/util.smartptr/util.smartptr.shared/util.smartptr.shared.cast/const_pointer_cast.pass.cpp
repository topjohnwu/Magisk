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

// template<class T, class U> shared_ptr<T> const_pointer_cast(const shared_ptr<U>& r);

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
        const std::shared_ptr<const A> pA(new A);
        std::shared_ptr<A> pB = std::const_pointer_cast<A>(pA);
        assert(pB.get() == pA.get());
        assert(!pB.owner_before(pA) && !pA.owner_before(pB));
    }
    {
        const std::shared_ptr<const A> pA;
        std::shared_ptr<A> pB = std::const_pointer_cast<A>(pA);
        assert(pB.get() == pA.get());
        assert(!pB.owner_before(pA) && !pA.owner_before(pB));
    }
}
