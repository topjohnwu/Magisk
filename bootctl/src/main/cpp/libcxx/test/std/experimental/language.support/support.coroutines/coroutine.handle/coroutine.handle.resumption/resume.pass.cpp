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

// void operator()()
// void resume()

#include <experimental/coroutine>
#include <type_traits>
#include <memory>
#include <utility>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

namespace coro = std::experimental;


template <class H>
auto has_resume_imp(H&& h, int) -> decltype(h.resume(), std::true_type{});
template <class H>
auto has_resume_imp(H&&, long) -> std::false_type;

template <class H>
constexpr bool has_resume() {
  return decltype(has_resume_imp(std::declval<H>(), 0))::value;
}


template <class H>
auto has_call_operator_imp(H&& h, int) -> decltype(h(), std::true_type{});
template <class H>
auto has_call_operator_imp(H&&, long) -> std::false_type;

template <class H>
constexpr bool has_call_operator() {
  return decltype(has_call_operator_imp(std::declval<H>(), 0))::value;
}

template <class Promise>
void do_test(coro::coroutine_handle<Promise>&& H) {
  using HType = coro::coroutine_handle<Promise>;
  // FIXME Add a runtime test
  {
    ASSERT_SAME_TYPE(decltype(H.resume()), void);
    ASSERT_SAME_TYPE(decltype(H()), void);
    LIBCPP_ASSERT_NOT_NOEXCEPT(H.resume());
    LIBCPP_ASSERT_NOT_NOEXCEPT(H());
    static_assert(has_resume<HType&>(), "");
    static_assert(has_resume<HType&&>(), "");
    static_assert(has_call_operator<HType&>(), "");
    static_assert(has_call_operator<HType&&>(), "");
  }
  {
    static_assert(!has_resume<HType const&>(), "");
    static_assert(!has_resume<HType const&&>(), "");
    static_assert(!has_call_operator<HType const&>(), "");
    static_assert(!has_call_operator<HType const&&>(), "");
  }
}

int main()
{
  do_test(coro::coroutine_handle<>{});
  do_test(coro::coroutine_handle<int>{});
}
