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

// mem_fun_ref_t

#include <functional>
#include <type_traits>
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
    typedef std::mem_fun_ref_t<char, A> F;
    static_assert((std::is_base_of<std::unary_function<A, char>, F>::value), "");
    const F f(&A::a1);
    A a;
    assert(f(a) == 5);
}
