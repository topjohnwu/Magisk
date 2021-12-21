//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// unique_ptr

// Test swap

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "deleter_types.h"

struct A
{
    int state_;
    static int count;
    A() : state_(0) {++count;}
    explicit A(int i) : state_(i) {++count;}
    A(const A& a) : state_(a.state_) {++count;}
    A& operator=(const A& a) {state_ = a.state_; return *this;}
    ~A() {--count;}

    friend bool operator==(const A& x, const A& y)
        {return x.state_ == y.state_;}
};

int A::count = 0;

template <class T>
struct NonSwappableDeleter {
  explicit NonSwappableDeleter(int) {}
  NonSwappableDeleter& operator=(NonSwappableDeleter const&) { return *this; }
  void operator()(T*) const {}
private:
  NonSwappableDeleter(NonSwappableDeleter const&);

};

int main()
{
    {
    A* p1 = new A(1);
    std::unique_ptr<A, Deleter<A> > s1(p1, Deleter<A>(1));
    A* p2 = new A(2);
    std::unique_ptr<A, Deleter<A> > s2(p2, Deleter<A>(2));
    assert(s1.get() == p1);
    assert(*s1 == A(1));
    assert(s1.get_deleter().state() == 1);
    assert(s2.get() == p2);
    assert(*s2 == A(2));
    assert(s2.get_deleter().state() == 2);
    swap(s1, s2);
    assert(s1.get() == p2);
    assert(*s1 == A(2));
    assert(s1.get_deleter().state() == 2);
    assert(s2.get() == p1);
    assert(*s2 == A(1));
    assert(s2.get_deleter().state() == 1);
    assert(A::count == 2);
    }
    assert(A::count == 0);
    {
    A* p1 = new A[3];
    std::unique_ptr<A[], Deleter<A[]> > s1(p1, Deleter<A[]>(1));
    A* p2 = new A[3];
    std::unique_ptr<A[], Deleter<A[]> > s2(p2, Deleter<A[]>(2));
    assert(s1.get() == p1);
    assert(s1.get_deleter().state() == 1);
    assert(s2.get() == p2);
    assert(s2.get_deleter().state() == 2);
    swap(s1, s2);
    assert(s1.get() == p2);
    assert(s1.get_deleter().state() == 2);
    assert(s2.get() == p1);
    assert(s2.get_deleter().state() == 1);
    assert(A::count == 6);
    }
    assert(A::count == 0);
#if TEST_STD_VER >= 11
    {
        // test that unique_ptr's specialized swap is disabled when the deleter
        // is non-swappable. Instead we should pick up the generic swap(T, T)
        // and perform 3 move constructions.
        typedef NonSwappableDeleter<int> D;
        D  d(42);
        int x = 42;
        int y = 43;
        std::unique_ptr<int, D&> p(&x, d);
        std::unique_ptr<int, D&> p2(&y, d);
        std::swap(p, p2);
    }
#endif
}
