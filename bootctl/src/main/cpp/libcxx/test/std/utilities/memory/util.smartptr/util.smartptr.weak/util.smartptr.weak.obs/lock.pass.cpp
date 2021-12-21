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

// shared_ptr<T> lock() const;

#include <memory>
#include <cassert>

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
    {
        std::weak_ptr<A> wp;
        std::shared_ptr<A> sp = wp.lock();
        assert(sp.use_count() == 0);
        assert(sp.get() == 0);
        assert(A::count == 0);
    }
    {
        std::shared_ptr<A> sp0(new A);
        std::weak_ptr<A> wp(sp0);
        std::shared_ptr<A> sp = wp.lock();
        assert(sp.use_count() == 2);
        assert(sp.get() == sp0.get());
        assert(A::count == 1);
    }
    assert(A::count == 0);
    {
        std::shared_ptr<A> sp0(new A);
        std::weak_ptr<A> wp(sp0);
        sp0.reset();
        std::shared_ptr<A> sp = wp.lock();
        assert(sp.use_count() == 0);
        assert(sp.get() == 0);
        assert(A::count == 0);
    }
    assert(A::count == 0);
}
