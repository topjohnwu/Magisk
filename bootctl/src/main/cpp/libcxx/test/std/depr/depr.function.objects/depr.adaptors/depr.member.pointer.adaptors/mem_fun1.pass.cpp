//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// REQUIRES: c++98 || c++03 || c++11 || c++14

// template<Returnable S, ClassType T, CopyConstructible A>
//   mem_fun1_t<S,T,A>
//   mem_fun(S (T::*f)(A));

#include <functional>
#include <cassert>

struct A
{
    char a1() {return 5;}
    short a2(int i) {return short(i+1);}
    int a3() const {return 1;}
    double a4(unsigned i) const {return i-1;}
};

int main()
{
    A a;
    assert(std::mem_fun(&A::a2)(&a, 5) == 6);
}
