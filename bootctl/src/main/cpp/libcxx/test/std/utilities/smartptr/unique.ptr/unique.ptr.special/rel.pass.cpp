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

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator< (const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator> (const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator<=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator>=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

#include <memory>
#include <cassert>

#include "deleter_types.h"

struct A
{
    static int count;
    A() {++count;}
    A(const A&) {++count;}
    virtual ~A() {--count;}
};

int A::count = 0;

struct B
    : public A
{
    static int count;
    B() {++count;}
    B(const B&) {++count;}
    virtual ~B() {--count;}
};

int B::count = 0;

int main()
{
    {
    const std::unique_ptr<A, Deleter<A> > p1(new A);
    const std::unique_ptr<A, Deleter<A> > p2(new A);
    assert((p1 < p2) == !(p1 > p2));
    assert((p1 < p2) == (p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
    {
    const std::unique_ptr<A, Deleter<A> > p1(new A);
    const std::unique_ptr<B, Deleter<B> > p2(new B);
    assert((p1 < p2) == !(p1 > p2));
    assert((p1 < p2) == (p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
    {
    const std::unique_ptr<A[], Deleter<A[]> > p1(new A[3]);
    const std::unique_ptr<A[], Deleter<A[]> > p2(new A[3]);
    assert((p1 < p2) == !(p1 > p2));
    assert((p1 < p2) == (p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
    {
    const std::unique_ptr<A[], Deleter<A[]> > p1(new A[3]);
    const std::unique_ptr<B[], Deleter<B[]> > p2(new B[3]);
    assert((p1 < p2) == !(p1 > p2));
    assert((p1 < p2) == (p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
    {
    const std::unique_ptr<A, Deleter<A> > p1;
    const std::unique_ptr<A, Deleter<A> > p2;
    assert((p1 < p2) == (p1 > p2));
    assert((p1 < p2) == !(p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
    {
    const std::unique_ptr<A, Deleter<A> > p1;
    const std::unique_ptr<B, Deleter<B> > p2;
    assert((p1 < p2) == (p1 > p2));
    assert((p1 < p2) == !(p1 <= p2));
    assert((p1 < p2) == !(p1 >= p2));
    }
}
