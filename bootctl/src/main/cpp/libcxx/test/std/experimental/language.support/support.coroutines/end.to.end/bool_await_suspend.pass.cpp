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

// See https://bugs.llvm.org/show_bug.cgi?id=33271
// UNSUPPORTED: ubsan

#include <experimental/coroutine>
#include <cassert>

using namespace std::experimental;

struct coro_t {
  struct promise_type {
    coro_t get_return_object() {
      return coroutine_handle<promise_type>::from_promise(*this);
    }
    suspend_never initial_suspend() { return {}; }
    suspend_never final_suspend() { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
  coro_t(coroutine_handle<promise_type> hh) : h(hh) {}
  coroutine_handle<promise_type> h;
};

struct NoSuspend {
  bool await_ready() { return false; }
  void await_resume() {}
  template <typename F> bool await_suspend(F) { return false; }
};

struct DoSuspend {
  bool await_ready() { return false; }
  void await_resume() {}
  template <typename F> bool await_suspend(F) { return true; }
};

bool f_started, f_resumed = false;
coro_t f() {
  f_started = true;
  co_await DoSuspend{};
  f_resumed = true;
}

bool g_started, g_resumed = false;
coro_t g() {
  g_started = true;
  co_await NoSuspend{};
  g_resumed = true;
}

int main() {
  assert(!f_started && !f_resumed && !g_started && !g_resumed);
  auto fret = f();
  assert(f_started && !f_resumed);
  fret.h.destroy();
  assert(f_started && !f_resumed);
  g();
  assert(g_started && g_resumed);
}
