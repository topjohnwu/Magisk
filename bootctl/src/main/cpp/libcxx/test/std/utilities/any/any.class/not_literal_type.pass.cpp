//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <any>

// [Note any is a not a literal type --end note]

#include <any>
#include <type_traits>

int main () {
    static_assert(!std::is_literal_type<std::any>::value, "");
}
