//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <experimental/type_traits>

#include <experimental/type_traits>
#include <string>

#include "test_macros.h"

namespace ex = std::experimental;

template <typename T>
  using copy_assign_t = decltype(std::declval<T&>() = std::declval<T const &>());

struct not_assignable {
    not_assignable & operator=(const not_assignable&) = delete;
};

template <typename T, bool b>
void test() {
    static_assert( b == ex::is_detected  <copy_assign_t, T>::value, "" );
    static_assert( b == ex::is_detected_v<copy_assign_t, T>, "" );
}

int main () {
    test<int, true>();
    test<std::string, true>();
    test<not_assignable, false>();
}
