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

// template<class Y> weak_ptr(const weak_ptr<Y>& r);
// template<class Y> weak_ptr(weak_ptr<Y> &&r);

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

int main()
{
    static_assert(( std::is_convertible<std::weak_ptr<A>, std::weak_ptr<B> >::value), "");
    static_assert((!std::is_convertible<std::weak_ptr<B>, std::weak_ptr<A> >::value), "");
    static_assert((!std::is_convertible<std::weak_ptr<A>, std::weak_ptr<C> >::value), "");
    {
        const std::weak_ptr<A> pA(std::shared_ptr<A>(new A));
        assert(pA.use_count() == 0);
        assert(B::count == 0);
        assert(A::count == 0);
        {
            std::weak_ptr<B> pB(pA);
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
    {
        std::weak_ptr<A> pA;
        assert(pA.use_count() == 0);
        assert(B::count == 0);
        assert(A::count == 0);
        {
            std::weak_ptr<B> pB(pA);
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

    {
        std::shared_ptr<A> ps(new A);
        std::weak_ptr<A> pA = source(ps);
        std::weak_ptr<B> pB(std::move(pA));
        assert(pB.use_count() == 1);
    }
    assert(B::count == 0);
    assert(A::count == 0);
}
