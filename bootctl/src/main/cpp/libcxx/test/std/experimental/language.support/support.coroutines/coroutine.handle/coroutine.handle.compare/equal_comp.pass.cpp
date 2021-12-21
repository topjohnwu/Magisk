// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <experimental/coroutine>

// template <class Promise = void>
// struct coroutine_handle;

// bool operator==(coroutine_handle<>, coroutine_handle<>) noexcept
// bool operator!=(coroutine_handle<>, coroutine_handle<>) noexcept

#include <experimental/coroutine>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

namespace coro = std::experimental;

template <class C>
void do_test(uintptr_t LHSVal, uintptr_t RHSVal) {
  const C LHS = C::from_address(reinterpret_cast<void*>(LHSVal));
  const C RHS = C::from_address(reinterpret_cast<void*>(RHSVal));
  const bool ExpectIsEqual = (LHSVal == RHSVal);
  assert((LHS == RHS) == ExpectIsEqual);
  assert((RHS == LHS) == ExpectIsEqual);
  assert((LHS != RHS) == !ExpectIsEqual);
  assert((RHS != LHS) == !ExpectIsEqual);
  {
    static_assert(noexcept(LHS == RHS), "");
    static_assert(noexcept(LHS != RHS), "");
    ASSERT_SAME_TYPE(decltype(LHS == RHS), bool);
    ASSERT_SAME_TYPE(decltype(LHS != RHS), bool);
  }
}

int main()
{
  std::pair<uintptr_t, uintptr_t> const TestCases[] = {
      {0, 0},
      {16, 16},
      {0, 16},
      {16, 0}
  };
  for (auto& TC : TestCases) {
    do_test<coro::coroutine_handle<>>(TC.first, TC.second);
    do_test<coro::coroutine_handle<int>>(TC.first, TC.second);
  }
}
