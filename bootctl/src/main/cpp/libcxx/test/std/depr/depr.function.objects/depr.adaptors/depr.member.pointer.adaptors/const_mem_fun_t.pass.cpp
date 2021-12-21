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

// const_mem_fun_t

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
    typedef std::const_mem_fun_t<int, A> F;
    static_assert((std::is_base_of<std::unary_function<const A*, int>, F>::value), "");
    const F f(&A::a3);
    const A a = A();
    assert(f(&a) == 1);
}
