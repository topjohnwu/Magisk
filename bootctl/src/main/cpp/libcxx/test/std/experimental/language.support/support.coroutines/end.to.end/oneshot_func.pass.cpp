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
#include <vector>
#include <cassert>

using namespace std::experimental;

// This file tests, one shot, movable std::function like thing using coroutine
// for compile-time type erasure and unerasure.

template <typename R> struct func {
  struct promise_type {
    R result;
    func get_return_object() { return {this}; }
    suspend_always initial_suspend() { return {}; }
    suspend_always final_suspend() { return {}; }
    void return_value(R v) { result = v; }
    void unhandled_exception() {}
  };

  R operator()() {
    h.resume();
    R result = h.promise().result;
    h.destroy();
    h = nullptr;
    return result;
  };

  func() {}
  func(func &&rhs) : h(rhs.h) { rhs.h = nullptr; }
  func(func const &) = delete;

  func &operator=(func &&rhs) {
    if (this != &rhs) {
      if (h)
        h.destroy();
      h = rhs.h;
      rhs.h = nullptr;
    }
    return *this;
  }

  template <typename F> static func Create(F f) { co_return f(); }

  template <typename F> func(F f) : func(Create(f)) {}

  ~func() {
    if (h)
      h.destroy();
  }

private:
  func(promise_type *promise)
      : h(coroutine_handle<promise_type>::from_promise(*promise)) {}
  coroutine_handle<promise_type> h;
};

std::vector<int> yielded_values = {};
int yield(int x) { yielded_values.push_back(x); return x + 1; }
float fyield(int x) { yielded_values.push_back(x); return static_cast<float>(x + 2); }

void Do1(func<int> f) { yield(f()); }
void Do2(func<double> f) { yield(static_cast<int>(f())); }

int main() {
  Do1([] { return yield(43); });
  assert((yielded_values == std::vector<int>{43, 44}));

  yielded_values = {};
  Do2([] { return fyield(44); });
  assert((yielded_values == std::vector<int>{44, 46}));
}
