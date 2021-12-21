// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <variant>

// template <class ...Types> class variant;

// template <class T>
// variant& operator=(T&&) noexcept(see below);

#include <cassert>
#include <string>
#include <type_traits>
#include <variant>

#include "test_macros.h"
#include "variant_test_helpers.hpp"

namespace MetaHelpers {

struct Dummy {
  Dummy() = default;
};

struct ThrowsCtorT {
  ThrowsCtorT(int) noexcept(false) {}
  ThrowsCtorT &operator=(int) noexcept { return *this; }
};

struct ThrowsAssignT {
  ThrowsAssignT(int) noexcept {}
  ThrowsAssignT &operator=(int) noexcept(false) { return *this; }
};

struct NoThrowT {
  NoThrowT(int) noexcept {}
  NoThrowT &operator=(int) noexcept { return *this; }
};

} // namespace MetaHelpers

namespace RuntimeHelpers {
#ifndef TEST_HAS_NO_EXCEPTIONS

struct ThrowsCtorT {
  int value;
  ThrowsCtorT() : value(0) {}
  ThrowsCtorT(int) noexcept(false) { throw 42; }
  ThrowsCtorT &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct MoveCrashes {
  int value;
  MoveCrashes(int v = 0) noexcept : value{v} {}
  MoveCrashes(MoveCrashes &&) noexcept { assert(false); }
  MoveCrashes &operator=(MoveCrashes &&) noexcept { assert(false); return *this; }
  MoveCrashes &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct ThrowsCtorTandMove {
  int value;
  ThrowsCtorTandMove() : value(0) {}
  ThrowsCtorTandMove(int) noexcept(false) { throw 42; }
  ThrowsCtorTandMove(ThrowsCtorTandMove &&) noexcept(false) { assert(false); }
  ThrowsCtorTandMove &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

struct ThrowsAssignT {
  int value;
  ThrowsAssignT() : value(0) {}
  ThrowsAssignT(int v) noexcept : value(v) {}
  ThrowsAssignT &operator=(int) noexcept(false) { throw 42; }
};

struct NoThrowT {
  int value;
  NoThrowT() : value(0) {}
  NoThrowT(int v) noexcept : value(v) {}
  NoThrowT &operator=(int v) noexcept {
    value = v;
    return *this;
  }
};

#endif // !defined(TEST_HAS_NO_EXCEPTIONS)
} // namespace RuntimeHelpers

void test_T_assignment_noexcept() {
  using namespace MetaHelpers;
  {
    using V = std::variant<Dummy, NoThrowT>;
    static_assert(std::is_nothrow_assignable<V, int>::value, "");
  }
  {
    using V = std::variant<Dummy, ThrowsCtorT>;
    static_assert(!std::is_nothrow_assignable<V, int>::value, "");
  }
  {
    using V = std::variant<Dummy, ThrowsAssignT>;
    static_assert(!std::is_nothrow_assignable<V, int>::value, "");
  }
}

void test_T_assignment_sfinae() {
  {
    using V = std::variant<long, unsigned>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
  {
    using V = std::variant<std::string, std::string>;
    static_assert(!std::is_assignable<V, const char *>::value, "ambiguous");
  }
  {
    using V = std::variant<std::string, void *>;
    static_assert(!std::is_assignable<V, int>::value, "no matching operator=");
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int, int &&>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
  {
    using V = std::variant<int, const int &>;
    static_assert(!std::is_assignable<V, int>::value, "ambiguous");
  }
#endif // TEST_VARIANT_HAS_NO_REFERENCES
}

void test_T_assignment_basic() {
  {
    std::variant<int> v(43);
    v = 42;
    assert(v.index() == 0);
    assert(std::get<0>(v) == 42);
  }
  {
    std::variant<int, long> v(43l);
    v = 42;
    assert(v.index() == 0);
    assert(std::get<0>(v) == 42);
    v = 43l;
    assert(v.index() == 1);
    assert(std::get<1>(v) == 43);
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &, int &&, long>;
    int x = 42;
    V v(43l);
    v = x;
    assert(v.index() == 0);
    assert(&std::get<0>(v) == &x);
    v = std::move(x);
    assert(v.index() == 1);
    assert(&std::get<1>(v) == &x);
    // 'long' is selected by FUN(const int &) since 'const int &' cannot bind
    // to 'int&'.
    const int &cx = x;
    v = cx;
    assert(v.index() == 2);
    assert(std::get<2>(v) == 42);
  }
#endif // TEST_VARIANT_HAS_NO_REFERENCES
}

void test_T_assignment_performs_construction() {
  using namespace RuntimeHelpers;
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = std::variant<std::string, ThrowsCtorT>;
    V v(std::in_place_type<std::string>, "hello");
    try {
      v = 42;
      assert(false);
    } catch (...) { /* ... */
    }
    assert(v.index() == 0);
    assert(std::get<0>(v) == "hello");
  }
  {
    using V = std::variant<ThrowsAssignT, std::string>;
    V v(std::in_place_type<std::string>, "hello");
    v = 42;
    assert(v.index() == 0);
    assert(std::get<0>(v).value == 42);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

void test_T_assignment_performs_assignment() {
  using namespace RuntimeHelpers;
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = std::variant<ThrowsCtorT>;
    V v;
    v = 42;
    assert(v.index() == 0);
    assert(std::get<0>(v).value == 42);
  }
  {
    using V = std::variant<ThrowsCtorT, std::string>;
    V v;
    v = 42;
    assert(v.index() == 0);
    assert(std::get<0>(v).value == 42);
  }
  {
    using V = std::variant<ThrowsAssignT>;
    V v(100);
    try {
      v = 42;
      assert(false);
    } catch (...) { /* ... */
    }
    assert(v.index() == 0);
    assert(std::get<0>(v).value == 100);
  }
  {
    using V = std::variant<std::string, ThrowsAssignT>;
    V v(100);
    try {
      v = 42;
      assert(false);
    } catch (...) { /* ... */
    }
    assert(v.index() == 1);
    assert(std::get<1>(v).value == 100);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

int main() {
  test_T_assignment_basic();
  test_T_assignment_performs_construction();
  test_T_assignment_performs_assignment();
  test_T_assignment_noexcept();
  test_T_assignment_sfinae();
}
