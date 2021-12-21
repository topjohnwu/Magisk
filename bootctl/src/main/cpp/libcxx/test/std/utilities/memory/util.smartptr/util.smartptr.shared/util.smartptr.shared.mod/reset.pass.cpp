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

// void reset();

#include <memory>
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
        std::shared_ptr<B> p(new B);
        p.reset();
        assert(A::count == 0);
        assert(B::count == 0);
        assert(p.use_count() == 0);
        assert(p.get() == 0);
    }
    assert(A::count == 0);
    {
        std::shared_ptr<B> p;
        p.reset();
        assert(A::count == 0);
        assert(B::count == 0);
        assert(p.use_count() == 0);
        assert(p.get() == 0);
    }
    assert(A::count == 0);
}
