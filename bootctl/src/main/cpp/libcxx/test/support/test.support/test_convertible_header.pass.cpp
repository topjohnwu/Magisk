//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// "support/test_convertible.hpp"

#include "test_convertible.hpp"

struct ImplicitDefault {
  ImplicitDefault() {}
};
static_assert(test_convertible<ImplicitDefault>(), "Must be convertible");

struct ExplicitDefault {
  explicit ExplicitDefault() {}
};
static_assert(!test_convertible<ExplicitDefault>(), "Must not be convertible");

struct ImplicitInt {
  ImplicitInt(int) {}
};
static_assert(test_convertible<ImplicitInt, int>(), "Must be convertible");

struct ExplicitInt {
  explicit ExplicitInt(int) {}
};
static_assert(!test_convertible<ExplicitInt, int>(), "Must not be convertible");

struct ImplicitCopy {
  ImplicitCopy(ImplicitCopy const&) {}
};
static_assert(test_convertible<ImplicitCopy, ImplicitCopy>(), "Must be convertible");

struct ExplicitCopy {
  explicit ExplicitCopy(ExplicitCopy const&) {}
};
static_assert(!test_convertible<ExplicitCopy, ExplicitCopy>(), "Must not be convertible");

struct ImplicitMove {
  ImplicitMove(ImplicitMove&&) {}
};
static_assert(test_convertible<ImplicitMove, ImplicitMove>(), "Must be convertible");

struct ExplicitMove {
  explicit ExplicitMove(ExplicitMove&&) {}
};
static_assert(!test_convertible<ExplicitMove, ExplicitMove>(), "Must not be convertible");

struct ImplicitArgs {
  ImplicitArgs(int, int, int) {}
};
static_assert(test_convertible<ImplicitArgs, int, int, int>(), "Must be convertible");

struct ExplicitArgs {
  explicit ExplicitArgs(int, int, int) {}
};
static_assert(!test_convertible<ExplicitArgs, int, int, int>(), "Must not be convertible");

int main() {
    // Nothing to do
}
