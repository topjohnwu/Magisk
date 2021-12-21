//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// unary_negate
//  deprecated in C++17

// UNSUPPORTED: clang-4.0
// UNSUPPORTED: c++98, c++03, c++11, c++14
// REQUIRES: verify-support

// MODULES_DEFINES: _LIBCPP_ENABLE_DEPRECATION_WARNINGS
#define _LIBCPP_ENABLE_DEPRECATION_WARNINGS

#include <functional>

#include "test_macros.h"

struct Predicate {
    typedef int argument_type;
    bool operator()(argument_type) const { return true; }
};

int main() {
    std::unary_negate<Predicate> f((Predicate())); // expected-error{{'unary_negate<Predicate>' is deprecated}}
    (void)f;
}
