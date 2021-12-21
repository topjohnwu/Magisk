// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: fcoroutines-ts

// RUN: %build -fcoroutines-ts
// RUN: %run

// A simple "breathing" test that checks that <experimental/coroutine>
// can be parsed and used in all dialects, including C++03 in order to match
// Clang's behavior.

#include <experimental/coroutine>

namespace coro = std::experimental::coroutines_v1;

coro::suspend_always sa;
coro::suspend_never sn;

struct MyFuture {
  struct promise_type {
    typedef coro::coroutine_handle<promise_type> HandleT;
    coro::suspend_never initial_suspend() { return sn; }
    coro::suspend_always final_suspend() { return sa; }
    coro::suspend_never yield_value(int) { return sn; }
    MyFuture get_return_object() {
      MyFuture f(HandleT::from_promise(*this));
      return f;
    }
    void return_void() {}
    void unhandled_exception() {}
  };
  typedef promise_type::HandleT HandleT;
  MyFuture() : p() {}
  MyFuture(HandleT h) : p(h) {}

  coro::coroutine_handle<promise_type> p;
};

MyFuture test_coro() {
  co_await sn;
  co_yield 42;
  co_return;
}

int main()
{
  MyFuture f = test_coro();
  while (!f.p.done())
    f.p.resume();
  f.p.destroy();
}
