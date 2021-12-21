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

// The following compilers don't generate constexpr special members correctly.
// XFAIL: clang-3.5, clang-3.6, clang-3.7, clang-3.8
// XFAIL: apple-clang-6, apple-clang-7, apple-clang-8.0

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <variant>

// template <class ...Types> class variant;

// variant& operator=(variant const&); // constexpr in C++20

#include <cassert>
#include <string>
#include <type_traits>
#include <variant>

#include "test_macros.h"

struct NoCopy {
  NoCopy(const NoCopy &) = delete;
  NoCopy &operator=(const NoCopy &) = default;
};

struct CopyOnly {
  CopyOnly(const CopyOnly &) = default;
  CopyOnly(CopyOnly &&) = delete;
  CopyOnly &operator=(const CopyOnly &) = default;
  CopyOnly &operator=(CopyOnly &&) = delete;
};

struct MoveOnly {
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&) = default;
  MoveOnly &operator=(const MoveOnly &) = default;
};

struct MoveOnlyNT {
  MoveOnlyNT(const MoveOnlyNT &) = delete;
  MoveOnlyNT(MoveOnlyNT &&) {}
  MoveOnlyNT &operator=(const MoveOnlyNT &) = default;
};

struct CopyAssign {
  static int alive;
  static int copy_construct;
  static int copy_assign;
  static int move_construct;
  static int move_assign;
  static void reset() {
    copy_construct = copy_assign = move_construct = move_assign = alive = 0;
  }
  CopyAssign(int v) : value(v) { ++alive; }
  CopyAssign(const CopyAssign &o) : value(o.value) {
    ++alive;
    ++copy_construct;
  }
  CopyAssign(CopyAssign &&o) noexcept : value(o.value) {
    o.value = -1;
    ++alive;
    ++move_construct;
  }
  CopyAssign &operator=(const CopyAssign &o) {
    value = o.value;
    ++copy_assign;
    return *this;
  }
  CopyAssign &operator=(CopyAssign &&o) noexcept {
    value = o.value;
    o.value = -1;
    ++move_assign;
    return *this;
  }
  ~CopyAssign() { --alive; }
  int value;
};

int CopyAssign::alive = 0;
int CopyAssign::copy_construct = 0;
int CopyAssign::copy_assign = 0;
int CopyAssign::move_construct = 0;
int CopyAssign::move_assign = 0;

struct CopyMaybeThrows {
  CopyMaybeThrows(const CopyMaybeThrows &);
  CopyMaybeThrows &operator=(const CopyMaybeThrows &);
};
struct CopyDoesThrow {
  CopyDoesThrow(const CopyDoesThrow &) noexcept(false);
  CopyDoesThrow &operator=(const CopyDoesThrow &) noexcept(false);
};


struct NTCopyAssign {
  constexpr NTCopyAssign(int v) : value(v) {}
  NTCopyAssign(const NTCopyAssign &) = default;
  NTCopyAssign(NTCopyAssign &&) = default;
  NTCopyAssign &operator=(const NTCopyAssign &that) {
    value = that.value;
    return *this;
  };
  NTCopyAssign &operator=(NTCopyAssign &&) = delete;
  int value;
};

static_assert(!std::is_trivially_copy_assignable<NTCopyAssign>::value, "");
static_assert(std::is_copy_assignable<NTCopyAssign>::value, "");

struct TCopyAssign {
  constexpr TCopyAssign(int v) : value(v) {}
  TCopyAssign(const TCopyAssign &) = default;
  TCopyAssign(TCopyAssign &&) = default;
  TCopyAssign &operator=(const TCopyAssign &) = default;
  TCopyAssign &operator=(TCopyAssign &&) = delete;
  int value;
};

static_assert(std::is_trivially_copy_assignable<TCopyAssign>::value, "");

struct TCopyAssignNTMoveAssign {
  constexpr TCopyAssignNTMoveAssign(int v) : value(v) {}
  TCopyAssignNTMoveAssign(const TCopyAssignNTMoveAssign &) = default;
  TCopyAssignNTMoveAssign(TCopyAssignNTMoveAssign &&) = default;
  TCopyAssignNTMoveAssign &operator=(const TCopyAssignNTMoveAssign &) = default;
  TCopyAssignNTMoveAssign &operator=(TCopyAssignNTMoveAssign &&that) {
    value = that.value;
    that.value = -1;
    return *this;
  }
  int value;
};

static_assert(std::is_trivially_copy_assignable_v<TCopyAssignNTMoveAssign>, "");

#ifndef TEST_HAS_NO_EXCEPTIONS
struct CopyThrows {
  CopyThrows() = default;
  CopyThrows(const CopyThrows &) { throw 42; }
  CopyThrows &operator=(const CopyThrows &) { throw 42; }
};

struct CopyCannotThrow {
  static int alive;
  CopyCannotThrow() { ++alive; }
  CopyCannotThrow(const CopyCannotThrow &) noexcept { ++alive; }
  CopyCannotThrow(CopyCannotThrow &&) noexcept { assert(false); }
  CopyCannotThrow &operator=(const CopyCannotThrow &) noexcept = default;
  CopyCannotThrow &operator=(CopyCannotThrow &&) noexcept { assert(false); return *this; }
};

int CopyCannotThrow::alive = 0;

struct MoveThrows {
  static int alive;
  MoveThrows() { ++alive; }
  MoveThrows(const MoveThrows &) { ++alive; }
  MoveThrows(MoveThrows &&) { throw 42; }
  MoveThrows &operator=(const MoveThrows &) { return *this; }
  MoveThrows &operator=(MoveThrows &&) { throw 42; }
  ~MoveThrows() { --alive; }
};

int MoveThrows::alive = 0;

struct MakeEmptyT {
  static int alive;
  MakeEmptyT() { ++alive; }
  MakeEmptyT(const MakeEmptyT &) {
    ++alive;
    // Don't throw from the copy constructor since variant's assignment
    // operator performs a copy before committing to the assignment.
  }
  MakeEmptyT(MakeEmptyT &&) { throw 42; }
  MakeEmptyT &operator=(const MakeEmptyT &) { throw 42; }
  MakeEmptyT &operator=(MakeEmptyT &&) { throw 42; }
  ~MakeEmptyT() { --alive; }
};

int MakeEmptyT::alive = 0;

template <class Variant> void makeEmpty(Variant &v) {
  Variant v2(std::in_place_type<MakeEmptyT>);
  try {
    v = std::move(v2);
    assert(false);
  } catch (...) {
    assert(v.valueless_by_exception());
  }
}
#endif // TEST_HAS_NO_EXCEPTIONS

void test_copy_assignment_not_noexcept() {
  {
    using V = std::variant<CopyMaybeThrows>;
    static_assert(!std::is_nothrow_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, CopyDoesThrow>;
    static_assert(!std::is_nothrow_copy_assignable<V>::value, "");
  }
}

void test_copy_assignment_sfinae() {
  {
    using V = std::variant<int, long>;
    static_assert(std::is_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, CopyOnly>;
    static_assert(std::is_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, NoCopy>;
    static_assert(!std::is_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, MoveOnly>;
    static_assert(!std::is_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, MoveOnlyNT>;
    static_assert(!std::is_copy_assignable<V>::value, "");
  }

  // Make sure we properly propagate triviality (see P0602R4).
#if TEST_STD_VER > 17
  {
    using V = std::variant<int, long>;
    static_assert(std::is_trivially_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, NTCopyAssign>;
    static_assert(!std::is_trivially_copy_assignable<V>::value, "");
    static_assert(std::is_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, TCopyAssign>;
    static_assert(std::is_trivially_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, TCopyAssignNTMoveAssign>;
    static_assert(std::is_trivially_copy_assignable<V>::value, "");
  }
  {
    using V = std::variant<int, CopyOnly>;
    static_assert(std::is_trivially_copy_assignable<V>::value, "");
  }
#endif // > C++17
}

void test_copy_assignment_empty_empty() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  using MET = MakeEmptyT;
  {
    using V = std::variant<int, long, MET>;
    V v1(std::in_place_index<0>);
    makeEmpty(v1);
    V v2(std::in_place_index<0>);
    makeEmpty(v2);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.valueless_by_exception());
    assert(v1.index() == std::variant_npos);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

void test_copy_assignment_non_empty_empty() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  using MET = MakeEmptyT;
  {
    using V = std::variant<int, MET>;
    V v1(std::in_place_index<0>, 42);
    V v2(std::in_place_index<0>);
    makeEmpty(v2);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.valueless_by_exception());
    assert(v1.index() == std::variant_npos);
  }
  {
    using V = std::variant<int, MET, std::string>;
    V v1(std::in_place_index<2>, "hello");
    V v2(std::in_place_index<0>);
    makeEmpty(v2);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.valueless_by_exception());
    assert(v1.index() == std::variant_npos);
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

void test_copy_assignment_empty_non_empty() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  using MET = MakeEmptyT;
  {
    using V = std::variant<int, MET>;
    V v1(std::in_place_index<0>);
    makeEmpty(v1);
    V v2(std::in_place_index<0>, 42);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 0);
    assert(std::get<0>(v1) == 42);
  }
  {
    using V = std::variant<int, MET, std::string>;
    V v1(std::in_place_index<0>);
    makeEmpty(v1);
    V v2(std::in_place_type<std::string>, "hello");
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 2);
    assert(std::get<2>(v1) == "hello");
  }
#endif // TEST_HAS_NO_EXCEPTIONS
}

template <typename T> struct Result { size_t index; T value; };

void test_copy_assignment_same_index() {
  {
    using V = std::variant<int>;
    V v1(43);
    V v2(42);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 0);
    assert(std::get<0>(v1) == 42);
  }
  {
    using V = std::variant<int, long, unsigned>;
    V v1(43l);
    V v2(42l);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 1);
    assert(std::get<1>(v1) == 42);
  }
  {
    using V = std::variant<int, CopyAssign, unsigned>;
    V v1(std::in_place_type<CopyAssign>, 43);
    V v2(std::in_place_type<CopyAssign>, 42);
    CopyAssign::reset();
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 1);
    assert(std::get<1>(v1).value == 42);
    assert(CopyAssign::copy_construct == 0);
    assert(CopyAssign::move_construct == 0);
    assert(CopyAssign::copy_assign == 1);
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  using MET = MakeEmptyT;
  {
    using V = std::variant<int, MET, std::string>;
    V v1(std::in_place_type<MET>);
    MET &mref = std::get<1>(v1);
    V v2(std::in_place_type<MET>);
    try {
      v1 = v2;
      assert(false);
    } catch (...) {
    }
    assert(v1.index() == 1);
    assert(&std::get<1>(v1) == &mref);
  }
#endif // TEST_HAS_NO_EXCEPTIONS

  // Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
#if TEST_STD_VER > 17
  {
    struct {
      constexpr Result<int> operator()() const {
        using V = std::variant<int>;
        V v(43);
        V v2(42);
        v = v2;
        return {v.index(), std::get<0>(v)};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 0, "");
    static_assert(result.value == 42, "");
  }
  {
    struct {
      constexpr Result<long> operator()() const {
        using V = std::variant<int, long, unsigned>;
        V v(43l);
        V v2(42l);
        v = v2;
        return {v.index(), std::get<1>(v)};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42l, "");
  }
  {
    struct {
      constexpr Result<int> operator()() const {
        using V = std::variant<int, TCopyAssign, unsigned>;
        V v(std::in_place_type<TCopyAssign>, 43);
        V v2(std::in_place_type<TCopyAssign>, 42);
        v = v2;
        return {v.index(), std::get<1>(v).value};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42, "");
  }
  {
    struct {
      constexpr Result<int> operator()() const {
        using V = std::variant<int, TCopyAssignNTMoveAssign, unsigned>;
        V v(std::in_place_type<TCopyAssignNTMoveAssign>, 43);
        V v2(std::in_place_type<TCopyAssignNTMoveAssign>, 42);
        v = v2;
        return {v.index(), std::get<1>(v).value};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42, "");
  }
#endif // > C++17
}

void test_copy_assignment_different_index() {
  {
    using V = std::variant<int, long, unsigned>;
    V v1(43);
    V v2(42l);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 1);
    assert(std::get<1>(v1) == 42);
  }
  {
    using V = std::variant<int, CopyAssign, unsigned>;
    CopyAssign::reset();
    V v1(std::in_place_type<unsigned>, 43);
    V v2(std::in_place_type<CopyAssign>, 42);
    assert(CopyAssign::copy_construct == 0);
    assert(CopyAssign::move_construct == 0);
    assert(CopyAssign::alive == 1);
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 1);
    assert(std::get<1>(v1).value == 42);
    assert(CopyAssign::alive == 2);
    assert(CopyAssign::copy_construct == 1);
    assert(CopyAssign::move_construct == 1);
    assert(CopyAssign::copy_assign == 0);
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = std::variant<int, CopyThrows, std::string>;
    V v1(std::in_place_type<std::string>, "hello");
    V v2(std::in_place_type<CopyThrows>);
    try {
      v1 = v2;
      assert(false);
    } catch (...) { /* ... */
    }
    // Test that copy construction is used directly if move construction may throw,
    // resulting in a valueless variant if copy throws.
    assert(v1.valueless_by_exception());
  }
  {
    using V = std::variant<int, MoveThrows, std::string>;
    V v1(std::in_place_type<std::string>, "hello");
    V v2(std::in_place_type<MoveThrows>);
    assert(MoveThrows::alive == 1);
    // Test that copy construction is used directly if move construction may throw.
    v1 = v2;
    assert(v1.index() == 1);
    assert(v2.index() == 1);
    assert(MoveThrows::alive == 2);
  }
  {
    // Test that direct copy construction is preferred when it cannot throw.
    using V = std::variant<int, CopyCannotThrow, std::string>;
    V v1(std::in_place_type<std::string>, "hello");
    V v2(std::in_place_type<CopyCannotThrow>);
    assert(CopyCannotThrow::alive == 1);
    v1 = v2;
    assert(v1.index() == 1);
    assert(v2.index() == 1);
    assert(CopyCannotThrow::alive == 2);
  }
  {
    using V = std::variant<int, CopyThrows, std::string>;
    V v1(std::in_place_type<CopyThrows>);
    V v2(std::in_place_type<std::string>, "hello");
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 2);
    assert(std::get<2>(v1) == "hello");
    assert(v2.index() == 2);
    assert(std::get<2>(v2) == "hello");
  }
  {
    using V = std::variant<int, MoveThrows, std::string>;
    V v1(std::in_place_type<MoveThrows>);
    V v2(std::in_place_type<std::string>, "hello");
    V &vref = (v1 = v2);
    assert(&vref == &v1);
    assert(v1.index() == 2);
    assert(std::get<2>(v1) == "hello");
    assert(v2.index() == 2);
    assert(std::get<2>(v2) == "hello");
  }
#endif // TEST_HAS_NO_EXCEPTIONS

  // Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
#if TEST_STD_VER > 17
  {
    struct {
      constexpr Result<long> operator()() const {
        using V = std::variant<int, long, unsigned>;
        V v(43);
        V v2(42l);
        v = v2;
        return {v.index(), std::get<1>(v)};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42l, "");
  }
  {
    struct {
      constexpr Result<int> operator()() const {
        using V = std::variant<int, TCopyAssign, unsigned>;
        V v(std::in_place_type<unsigned>, 43);
        V v2(std::in_place_type<TCopyAssign>, 42);
        v = v2;
        return {v.index(), std::get<1>(v).value};
      }
    } test;
    constexpr auto result = test();
    static_assert(result.index == 1, "");
    static_assert(result.value == 42, "");
  }
#endif // > C++17
}

template <size_t NewIdx, class ValueType>
constexpr bool test_constexpr_assign_imp(
    std::variant<long, void*, int>&& v, ValueType&& new_value)
{
  const std::variant<long, void*, int> cp(
      std::forward<ValueType>(new_value));
  v = cp;
  return v.index() == NewIdx &&
        std::get<NewIdx>(v) == std::get<NewIdx>(cp);
}

void test_constexpr_copy_assignment() {
  // Make sure we properly propagate triviality, which implies constexpr-ness (see P0602R4).
#if TEST_STD_VER > 17
  using V = std::variant<long, void*, int>;
  static_assert(std::is_trivially_copyable<V>::value, "");
  static_assert(std::is_trivially_copy_assignable<V>::value, "");
  static_assert(test_constexpr_assign_imp<0>(V(42l), 101l), "");
  static_assert(test_constexpr_assign_imp<0>(V(nullptr), 101l), "");
  static_assert(test_constexpr_assign_imp<1>(V(42l), nullptr), "");
  static_assert(test_constexpr_assign_imp<2>(V(42l), 101), "");
#endif // > C++17
}

int main() {
  test_copy_assignment_empty_empty();
  test_copy_assignment_non_empty_empty();
  test_copy_assignment_empty_non_empty();
  test_copy_assignment_same_index();
  test_copy_assignment_different_index();
  test_copy_assignment_sfinae();
  test_copy_assignment_not_noexcept();
  test_constexpr_copy_assignment();
}
