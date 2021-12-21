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

namespace coro = std::experimental;

template <class T, class = typename T::promise_type>
constexpr bool has_promise_type(int) { return true; }
template <class>
constexpr bool has_promise_type(long) { return false; }
template <class T>
constexpr bool has_promise_type() { return has_promise_type<T>(0); }

struct A {
  using promise_type = A*;
};

struct B {};
struct C {};
struct D {
private:
  using promise_type = void;
};
struct E {};

namespace std { namespace experimental {
  template <>
  struct coroutine_traits<::A, int> {
    using promise_type = int*;
  };
  template <class ...Args>
  struct coroutine_traits<::B, Args...> {
    using promise_type = B*;
  };
  template <>
  struct coroutine_traits<::C> {
    using promise_type = void;
  };
}}

template <class Expect, class T, class ...Args>
void check_type() {
  using Traits = coro::coroutine_traits<T, Args...>;
  static_assert(has_promise_type<Traits>(), "");
  static_assert(std::is_same<typename Traits::promise_type, Expect>::value, "");
}

template <class T, class ...Args>
void check_no_type() {
  using Traits = coro::coroutine_traits<T, Args...>;
  static_assert(!has_promise_type<Traits>(), "");
}

int main()
{
  {
    check_type<A*, A>();
    check_type<int*, A, int>();
    check_type<B*, B>();
    check_type<void, C>();
  }
  {
    check_no_type<D>();
    check_no_type<E>();
    check_no_type<C, int>();
  }
}
