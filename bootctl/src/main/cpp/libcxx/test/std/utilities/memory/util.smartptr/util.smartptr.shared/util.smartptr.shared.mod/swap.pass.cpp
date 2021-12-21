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

// void swap(shared_ptr& r);

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
        {
            std::shared_ptr<A> p2(ptr2);
            p1.swap(p2);
            assert(p1.use_count() == 1);
            assert(p1.get() == ptr2);
            assert(p2.use_count() == 1);
            assert(p2.get() == ptr1);
            assert(A::count == 2);
        }
        assert(p1.use_count() == 1);
        assert(p1.get() == ptr2);
        assert(A::count == 1);
    }
    assert(A::count == 0);
    {
        A* ptr1 = new A;
        A* ptr2 = 0;
        std::shared_ptr<A> p1(ptr1);
        {
            std::shared_ptr<A> p2;
            p1.swap(p2);
            assert(p1.use_count() == 0);
            assert(p1.get() == ptr2);
            assert(p2.use_count() == 1);
            assert(p2.get() == ptr1);
            assert(A::count == 1);
        }
        assert(p1.use_count() == 0);
        assert(p1.get() == ptr2);
        assert(A::count == 0);
    }
    assert(A::count == 0);
    {
        A* ptr1 = 0;
        A* ptr2 = new A;
        std::shared_ptr<A> p1;
        {
            std::shared_ptr<A> p2(ptr2);
            p1.swap(p2);
            assert(p1.use_count() == 1);
            assert(p1.get() == ptr2);
            assert(p2.use_count() == 0);
            assert(p2.get() == ptr1);
            assert(A::count == 1);
        }
        assert(p1.use_count() == 1);
        assert(p1.get() == ptr2);
        assert(A::count == 1);
    }
    assert(A::count == 0);
    {
        A* ptr1 = 0;
        A* ptr2 = 0;
        std::shared_ptr<A> p1;
        {
            std::shared_ptr<A> p2;
            p1.swap(p2);
            assert(p1.use_count() == 0);
            assert(p1.get() == ptr2);
            assert(p2.use_count() == 0);
            assert(p2.get() == ptr1);
            assert(A::count == 0);
        }
        assert(p1.use_count() == 0);
        assert(p1.get() == ptr2);
        assert(A::count == 0);
    }
    assert(A::count == 0);
}
