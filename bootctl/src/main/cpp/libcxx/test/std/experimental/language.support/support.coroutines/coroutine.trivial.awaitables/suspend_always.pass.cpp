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

#include <experimental/coroutine>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

namespace coro = std::experimental;

using SuspendT = std::experimental::coroutines_v1::suspend_always;

TEST_SAFE_STATIC SuspendT safe_sa;
constexpr SuspendT constexpr_sa;

constexpr bool check_suspend_constexpr() {
  SuspendT s{};
  const SuspendT scopy(s); ((void)scopy);
  SuspendT smove(std::move(s)); ((void)smove);
  s = scopy;
  s = std::move(smove);
  return true;
}

int main()
{
  using H = coro::coroutine_handle<>;
  using S = SuspendT;
  H h{};
  S s{};
  S const& cs = s;
  {
    LIBCPP_STATIC_ASSERT(noexcept(s.await_ready()), "");
    static_assert(std::is_same<decltype(s.await_ready()), bool>::value, "");
    assert(s.await_ready() == false);
    assert(cs.await_ready() == false);
  }
  {
    LIBCPP_STATIC_ASSERT(noexcept(s.await_suspend(h)), "");
    static_assert(std::is_same<decltype(s.await_suspend(h)), void>::value, "");
    s.await_suspend(h);
    cs.await_suspend(h);
  }
  {
    LIBCPP_STATIC_ASSERT(noexcept(s.await_resume()), "");
    static_assert(std::is_same<decltype(s.await_resume()), void>::value, "");
    s.await_resume();
    cs.await_resume();
  }
  {
    static_assert(std::is_nothrow_default_constructible<S>::value, "");
    static_assert(std::is_nothrow_copy_constructible<S>::value, "");
    static_assert(std::is_nothrow_move_constructible<S>::value, "");
    static_assert(std::is_nothrow_copy_assignable<S>::value, "");
    static_assert(std::is_nothrow_move_assignable<S>::value, "");
    static_assert(std::is_trivially_copyable<S>::value, "");
    static_assert(check_suspend_constexpr(), "");
  }
  {
    // suppress unused warnings for the global constexpr test variable
    ((void)constexpr_sa);
  }
}
