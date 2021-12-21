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

// template<Returnable S, ClassType T>
//   const_mem_fun_ref_t<S,T>
//   mem_fun_ref(S (T::*f)() const);

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
    assert(std::mem_fun_ref(&A::a3)(a) == 1);
}
