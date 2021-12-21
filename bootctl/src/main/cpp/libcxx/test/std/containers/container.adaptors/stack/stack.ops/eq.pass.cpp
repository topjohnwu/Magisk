//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>

// template <class T, class Container>
//   bool operator==(const stack<T, Container>& x,const stack<T, Container>& y);
//
// template <class T, class Container>
//   bool operator!=(const stack<T, Container>& x,const stack<T, Container>& y);

#include <stack>
#include <cassert>

template <class C>
C
make(int n)
{
    C c;
    for (int i = 0; i < n; ++i)
        c.push(i);
    return c;
}

int main()
{
    std::stack<int> q1 = make<std::stack<int> >(5);
    std::stack<int> q2 = make<std::stack<int> >(10);
    std::stack<int> q1_save = q1;
    std::stack<int> q2_save = q2;
    assert(q1 == q1_save);
    assert(q1 != q2);
    assert(q2 == q2_save);
}
