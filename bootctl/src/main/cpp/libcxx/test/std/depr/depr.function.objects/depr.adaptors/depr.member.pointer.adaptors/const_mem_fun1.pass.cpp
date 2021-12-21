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
//   const_mem_fun1_t<S,T,A>
//   mem_fun(S (T::*f)(A) const);

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
    const A a = A();
    assert(std::mem_fun(&A::a4)(&a, 6) == 5);
}
