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
#include <cassert>

#include "test_macros.h"

using namespace std::experimental;

int alive = 0;
int ctor_called = 0;
int dtor_called = 0;
void reset() {
  assert(alive == 0);
  alive = 0;
  ctor_called = 0;
  dtor_called = 0;
}
struct Noisy {
  Noisy() { ++alive; ++ctor_called; }
  ~Noisy() { --alive; ++dtor_called; }
#if TEST_STD_VER > 14
  Noisy(Noisy const&) = delete;
#else
  // FIXME: This test depends on copy elision taking place in C++14
  // (pre-C++17 guaranteed copy elision)
  Noisy(Noisy const&);
#endif
};

struct Bug {
  bool await_ready() { return true; }
  void await_suspend(std::experimental::coroutine_handle<>) {}
  Noisy await_resume() { return {}; }
};
struct coro2 {
  struct promise_type {
    suspend_never initial_suspend() { return{}; }
    suspend_never final_suspend() { return{}; }
    coro2 get_return_object() { return{}; }
    void return_void() {}
    Bug yield_value(int) { return {}; }
    void unhandled_exception() {}
  };
};

// Checks that destructors are correctly invoked for the object returned by
// coawait.
coro2 a() {
  reset();
  {
    auto x = co_await Bug{};
    assert(alive == 1);
    assert(ctor_called == 1);
    assert(dtor_called == 0);
    ((void)x);
  }
  assert(alive == 0);
  assert(dtor_called == 1);
}

coro2 b() {
  reset();
  {
    (void)(co_await Bug{});
    assert(ctor_called == 1);
    assert(dtor_called == 1);
    assert(alive == 0);
  }
  assert(ctor_called == 1);
  assert(dtor_called == 1);
  assert(alive == 0);

}

coro2 c() {
  reset();
  {
    auto x = co_yield 42;
    assert(alive == 1);
    assert(ctor_called == 1);
    assert(dtor_called == 0);
  }
  assert(alive == 0);
  assert(ctor_called == 1);
  assert(dtor_called == 1);
}

coro2 d() {
  reset();
  {
    (void)(co_yield 42);
    assert(ctor_called == 1);
    assert(dtor_called == 1);
    assert(alive == 0);
  }
  assert(alive == 0);
  assert(ctor_called == 1);
  assert(dtor_called == 1);
}

int main() {
  a();
  b();
  c();
  d();
}
