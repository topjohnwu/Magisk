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

// void swap(weak_ptr& r);

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
        A* ptr1 = new A;
        A* ptr2 = new A;
        std::shared_ptr<A> p1(ptr1);
        std::weak_ptr<A> w1(p1);
        {
            std::shared_ptr<A> p2(ptr2);
            std::weak_ptr<A> w2(p2);
            w1.swap(w2);
            assert(w1.use_count() == 1);
            assert(w1.lock().get() == ptr2);
            assert(w2.use_count() == 1);
            assert(w2.lock().get() == ptr1);
            assert(A::count == 2);
        }
    }
    assert(A::count == 0);
}
