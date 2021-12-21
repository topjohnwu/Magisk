//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test rel_ops

#include <utility>
#include <cassert>

struct A
{
    int data_;

    explicit A(int data = -1) : data_(data) {}
};

inline
bool
operator == (const A& x, const A& y)
{
    return x.data_ == y.data_;
}

inline
bool
operator < (const A& x, const A& y)
{
    return x.data_ < y.data_;
}

int main()
{
    using namespace std::rel_ops;
    A a1(1);
    A a2(2);
    assert(a1 == a1);
    assert(a1 != a2);
    assert(a1 < a2);
    assert(a2 > a1);
    assert(a1 <= a1);
    assert(a1 <= a2);
    assert(a2 >= a2);
    assert(a2 >= a1);
}
