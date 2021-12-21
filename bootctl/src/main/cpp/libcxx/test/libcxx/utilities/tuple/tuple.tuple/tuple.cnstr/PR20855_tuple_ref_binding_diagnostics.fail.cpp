// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <tuple>

// See llvm.org/PR20855

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdangling-field"
#endif

#include <tuple>
#include <string>
#include "test_macros.h"

template <class Tp>
struct ConvertsTo {
  using RawTp = typename std::remove_cv< typename std::remove_reference<Tp>::type>::type;

  operator Tp() const {
    return static_cast<Tp>(value);
  }

  mutable RawTp value;
};

struct Base {};
struct Derived : Base {};

template <class T> struct CannotDeduce {
 using type = T;
};

template <class ...Args>
void F(typename CannotDeduce<std::tuple<Args...>>::type const&) {}


int main() {
#if TEST_HAS_BUILTIN_IDENTIFIER(__reference_binds_to_temporary)
  // Test that we emit our diagnostic from the library.
  // expected-error@tuple:* 8 {{"Attempted construction of reference element binds to a temporary whose lifetime has ended"}}

  // Good news everybody! Clang now diagnoses this for us!
  // expected-error@tuple:* 0+ {{reference member '__value_' binds to a temporary object whose lifetime would be shorter than the lifetime of the constructed object}}

  {
    F<int, const std::string&>(std::make_tuple(1, "abc")); // expected-note 1 {{requested here}}
  }
  {
    std::tuple<int, const std::string&> t(1, "a"); // expected-note 1 {{requested here}}
  }
  {
    F<int, const std::string&>(std::tuple<int, const std::string&>(1, "abc")); // expected-note 1 {{requested here}}
  }
  {
    ConvertsTo<int&> ct;
    std::tuple<const long&, int> t(ct, 42); // expected-note {{requested here}}
  }
  {
    ConvertsTo<int> ct;
    std::tuple<int const&, void*> t(ct, nullptr); // expected-note {{requested here}}
  }
  {
    ConvertsTo<Derived> ct;
    std::tuple<Base const&, int> t(ct, 42); // expected-note {{requested here}}
  }
  {
    std::allocator<void> alloc;
    std::tuple<std::string &&> t2("hello"); // expected-note {{requested here}}
    std::tuple<std::string &&> t3(std::allocator_arg, alloc, "hello"); // expected-note {{requested here}}
  }
#else
#error force failure
// expected-error@-1 {{force failure}}
#endif
}
