//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// constexpr optional(const optional<T>&& rhs);
//   If is_trivially_move_constructible_v<T> is true,
//    this constructor shall be a constexpr constructor.

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

struct S {
    constexpr S()   : v_(0) {}
    S(int v)        : v_(v) {}
    constexpr S(const S  &rhs) : v_(rhs.v_) {} // not trivially moveable
    constexpr S(const S &&rhs) : v_(rhs.v_) {} // not trivially moveable
    int v_;
    };


int main()
{
    static_assert (!std::is_trivially_move_constructible_v<S>, "" );
    constexpr std::optional<S> o1;
    constexpr std::optional<S> o2 = std::move(o1);  // not constexpr
}
