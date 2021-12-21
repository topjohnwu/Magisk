//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// UNSUPPORTED: clang-4.0
// UNSUPPORTED: c++98, c++03
// REQUIRES: verify-support

// MODULES_DEFINES: _LIBCPP_ENABLE_DEPRECATION_WARNINGS
// MODULES_DEFINES: _LIBCPP_ENABLE_CXX17_REMOVED_BINDERS
#define _LIBCPP_ENABLE_DEPRECATION_WARNINGS
#define _LIBCPP_ENABLE_CXX17_REMOVED_BINDERS

#include <functional>
#include <cassert>
#include "test_macros.h"

int identity(int v) { return v; }
int sum(int a, int b) { return a + b; }

struct Foo {
    int const_zero() const { return 0; }
    int const_identity(int v) const { return v; }
    int zero() { return 0; }
    int identity(int v) { return v; }
};

int main()
{
    typedef std::pointer_to_unary_function<int, int> PUF; // expected-error{{'pointer_to_unary_function<int, int>' is deprecated}}
    typedef std::pointer_to_binary_function<int, int, int> PBF; // expected-error{{'pointer_to_binary_function<int, int, int>' is deprecated}}
    std::ptr_fun<int, int>(identity); // expected-error{{'ptr_fun<int, int>' is deprecated}}
    std::ptr_fun<int, int, int>(sum); // expected-error{{'ptr_fun<int, int, int>' is deprecated}}

    typedef std::mem_fun_t<int, Foo> MFT0; // expected-error{{'mem_fun_t<int, Foo>' is deprecated}}
    typedef std::mem_fun1_t<int, Foo, int> MFT1; // expected-error{{'mem_fun1_t<int, Foo, int>' is deprecated}}
    typedef std::const_mem_fun_t<int, Foo> CMFT0; // expected-error{{'const_mem_fun_t<int, Foo>' is deprecated}}
    typedef std::const_mem_fun1_t<int, Foo, int> CMFT1; // expected-error{{'const_mem_fun1_t<int, Foo, int>' is deprecated}}
    std::mem_fun<int, Foo>(&Foo::zero); // expected-error{{'mem_fun<int, Foo>' is deprecated}}
    std::mem_fun<int, Foo, int>(&Foo::identity); // expected-error{{'mem_fun<int, Foo, int>' is deprecated}}
    std::mem_fun<int, Foo>(&Foo::const_zero); // expected-error{{'mem_fun<int, Foo>' is deprecated}}
    std::mem_fun<int, Foo, int>(&Foo::const_identity); // expected-error{{'mem_fun<int, Foo, int>' is deprecated}}

    typedef std::mem_fun_ref_t<int, Foo> MFR0; // expected-error{{'mem_fun_ref_t<int, Foo>' is deprecated}}
    typedef std::mem_fun1_ref_t<int, Foo, int> MFR1; // expected-error{{'mem_fun1_ref_t<int, Foo, int>' is deprecated}}
    typedef std::const_mem_fun_ref_t<int, Foo> CMFR0; // expected-error{{'const_mem_fun_ref_t<int, Foo>' is deprecated}}
    typedef std::const_mem_fun1_ref_t<int, Foo, int> CMFR1; // expected-error{{'const_mem_fun1_ref_t<int, Foo, int>' is deprecated}}
    std::mem_fun_ref<int, Foo>(&Foo::zero); // expected-error{{'mem_fun_ref<int, Foo>' is deprecated}}
    std::mem_fun_ref<int, Foo, int>(&Foo::identity); // expected-error{{'mem_fun_ref<int, Foo, int>' is deprecated}}
    std::mem_fun_ref<int, Foo>(&Foo::const_zero); // expected-error{{'mem_fun_ref<int, Foo>' is deprecated}}
    std::mem_fun_ref<int, Foo, int>(&Foo::const_identity); // expected-error{{'mem_fun_ref<int, Foo, int>' is deprecated}}
}
