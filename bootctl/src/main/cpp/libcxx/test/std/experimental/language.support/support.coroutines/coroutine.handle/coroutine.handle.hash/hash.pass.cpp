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

// namespace std {
//  template <class P> struct hash<experimental::coroutine_handle<P>>;
// }

#include <experimental/coroutine>
#include <type_traits>
#include <memory>
#include <utility>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

namespace coro = std::experimental;

template <class C>
void do_test(uintptr_t LHSVal, uintptr_t RHSVal) {
  const size_t ExpectLHS = std::hash<void*>{}(reinterpret_cast<void*>(LHSVal));
  const size_t ExpectRHS = std::hash<void*>{}(reinterpret_cast<void*>(RHSVal));
  const C LHS = C::from_address(reinterpret_cast<void*>(LHSVal));
  const C RHS = C::from_address(reinterpret_cast<void*>(RHSVal));
  const std::hash<C> h;
  // FIXME: libc++'s implementation hash's the result of LHS.address(), so we
  // expect that value. However this is not required.
  assert(h(LHS) == ExpectLHS);
  assert(h(RHS) == ExpectRHS);
  assert((h(LHS) == h(RHS)) == (LHSVal == RHSVal));
  {
    ASSERT_SAME_TYPE(decltype(h(LHS)), size_t);
    ASSERT_NOEXCEPT(std::hash<C>{}(LHS));
  }
}

int main()
{
  std::pair<uintptr_t, uintptr_t> const TestCases[] = {
      {0, 0},
      {0, 8},
      {8, 8},
      {8, 16}
  };
  for (auto& TC : TestCases) {
    do_test<coro::coroutine_handle<>>(TC.first, TC.second);
    do_test<coro::coroutine_handle<int>>(TC.first, TC.second);
  }
}
