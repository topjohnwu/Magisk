/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "android-base/expected.h"

#include <cstdio>
#include <memory>
#include <string>

#include <gtest/gtest.h>

using android::base::expected;
using android::base::unexpected;

typedef expected<int, int> exp_int;
typedef expected<double, double> exp_double;
typedef expected<std::string, std::string> exp_string;
typedef expected<std::pair<std::string, int>, int> exp_pair;
typedef expected<void, int> exp_void;

struct T {
  int a;
  int b;
  T() = default;
  T(int a, int b) noexcept : a(a), b(b) {}
};
bool operator==(const T& x, const T& y) {
  return x.a == y.a && x.b == y.b;
}
bool operator!=(const T& x, const T& y) {
  return x.a != y.a || x.b != y.b;
}

struct E {
    std::string message;
    int cause;
    E(const std::string& message, int cause) : message(message), cause(cause) {}
};

typedef expected<T,E> exp_complex;

TEST(Expected, testDefaultConstructible) {
  exp_int e;
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(0, e.value());

  exp_complex e2;
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(T(0,0), e2.value());

  exp_void e3;
  EXPECT_TRUE(e3.has_value());
}

TEST(Expected, testCopyConstructible) {
  exp_int e;
  exp_int e2 = e;

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(0, e.value());
  EXPECT_EQ(0, e2.value());

  exp_void e3;
  exp_void e4 = e3;
  EXPECT_TRUE(e3.has_value());
  EXPECT_TRUE(e4.has_value());
}

TEST(Expected, testMoveConstructible) {
  exp_int e;
  exp_int e2 = std::move(e);

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(0, e.value());
  EXPECT_EQ(0, e2.value());

  exp_string e3(std::string("hello"));
  exp_string e4 = std::move(e3);

  EXPECT_TRUE(e3.has_value());
  EXPECT_TRUE(e4.has_value());
  EXPECT_EQ("", e3.value()); // e3 is moved
  EXPECT_EQ("hello", e4.value());

  exp_void e5;
  exp_void e6 = std::move(e5);
  EXPECT_TRUE(e5.has_value());
  EXPECT_TRUE(e6.has_value());
}

TEST(Expected, testCopyConstructibleFromConvertibleType) {
  exp_double e = 3.3f;
  exp_int e2 = e;

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(3.3f, e.value());
  EXPECT_EQ(3, e2.value());
}

TEST(Expected, testMoveConstructibleFromConvertibleType) {
  exp_double e = 3.3f;
  exp_int e2 = std::move(e);

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(3.3f, e.value());
  EXPECT_EQ(3, e2.value());
}

TEST(Expected, testConstructibleFromValue) {
  exp_int e = 3;
  exp_double e2 = 5.5f;
  exp_string e3 = std::string("hello");
  exp_complex e4 = T(10, 20);
  exp_void e5 = {};

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_TRUE(e3.has_value());
  EXPECT_TRUE(e4.has_value());
  EXPECT_TRUE(e5.has_value());
  EXPECT_EQ(3, e.value());
  EXPECT_EQ(5.5f, e2.value());
  EXPECT_EQ("hello", e3.value());
  EXPECT_EQ(T(10,20), e4.value());
}

TEST(Expected, testConstructibleFromMovedValue) {
  std::string hello = "hello";
  exp_string e = std::move(hello);

  EXPECT_TRUE(e.has_value());
  EXPECT_EQ("hello", e.value());
  EXPECT_EQ("", hello);
}

TEST(Expected, testConstructibleFromConvertibleValue) {
  exp_int e = 3.3f; // double to int
  exp_string e2 = "hello"; // char* to std::string
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(3, e.value());

  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ("hello", e2.value());
}

TEST(Expected, testConstructibleFromUnexpected) {
  exp_int::unexpected_type unexp = unexpected(10);
  exp_int e = unexp;

  exp_double::unexpected_type unexp2 = unexpected(10.5f);
  exp_double e2 = unexp2;

  exp_string::unexpected_type unexp3 = unexpected(std::string("error"));
  exp_string e3 = unexp3;

  exp_void::unexpected_type unexp4 = unexpected(10);
  exp_void e4 = unexp4;

  EXPECT_FALSE(e.has_value());
  EXPECT_FALSE(e2.has_value());
  EXPECT_FALSE(e3.has_value());
  EXPECT_FALSE(e4.has_value());
  EXPECT_EQ(10, e.error());
  EXPECT_EQ(10.5f, e2.error());
  EXPECT_EQ("error", e3.error());
  EXPECT_EQ(10, e4.error());
}

TEST(Expected, testMoveConstructibleFromUnexpected) {
  exp_int e = unexpected(10);
  exp_double e2 = unexpected(10.5f);
  exp_string e3 = unexpected(std::string("error"));
  exp_void e4 = unexpected(10);

  EXPECT_FALSE(e.has_value());
  EXPECT_FALSE(e2.has_value());
  EXPECT_FALSE(e3.has_value());
  EXPECT_FALSE(e4.has_value());
  EXPECT_EQ(10, e.error());
  EXPECT_EQ(10.5f, e2.error());
  EXPECT_EQ("error", e3.error());
  EXPECT_EQ(10, e4.error());
}

TEST(Expected, testConstructibleByForwarding) {
  exp_string e(std::in_place, 5, 'a');
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ("aaaaa", e.value());

  exp_string e2({'a', 'b', 'c'});
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ("abc", e2.value());

  exp_pair e3({"hello", 30});
  EXPECT_TRUE(e3.has_value());
  EXPECT_EQ("hello",e3->first);
  EXPECT_EQ(30,e3->second);

  exp_void e4({});
  EXPECT_TRUE(e4.has_value());
}

TEST(Expected, testDestructible) {
  bool destroyed = false;
  struct T {
    bool* flag_;
    T(bool* flag) : flag_(flag) {}
    ~T() { *flag_ = true; }
  };
  {
    expected<T, int> exp = T(&destroyed);
  }
  EXPECT_TRUE(destroyed);
}

TEST(Expected, testAssignable) {
  exp_int e = 10;
  exp_int e2 = 20;
  e = e2;

  EXPECT_EQ(20, e.value());
  EXPECT_EQ(20, e2.value());

  exp_int e3 = 10;
  exp_int e4 = 20;
  e3 = std::move(e4);

  EXPECT_EQ(20, e3.value());
  EXPECT_EQ(20, e4.value());

  exp_void e5 = unexpected(10);
  ASSERT_FALSE(e5.has_value());
  exp_void e6;
  e5 = e6;

  EXPECT_TRUE(e5.has_value());
  EXPECT_TRUE(e6.has_value());
}

TEST(Expected, testAssignableFromValue) {
  exp_int e = 10;
  e = 20;
  EXPECT_EQ(20, e.value());

  exp_double e2 = 3.5f;
  e2 = 10.5f;
  EXPECT_EQ(10.5f, e2.value());

  exp_string e3 = "hello";
  e3 = "world";
  EXPECT_EQ("world", e3.value());

  exp_void e4 = unexpected(10);
  ASSERT_FALSE(e4.has_value());
  e4 = {};
  EXPECT_TRUE(e4.has_value());
}

TEST(Expected, testAssignableFromUnexpected) {
  exp_int e = 10;
  e = unexpected(30);
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(30, e.error());

  exp_double e2 = 3.5f;
  e2 = unexpected(10.5f);
  EXPECT_FALSE(e2.has_value());
  EXPECT_EQ(10.5f, e2.error());

  exp_string e3 = "hello";
  e3 = unexpected("world");
  EXPECT_FALSE(e3.has_value());
  EXPECT_EQ("world", e3.error());

  exp_void e4 = {};
  e4 = unexpected(10);
  EXPECT_FALSE(e4.has_value());
  EXPECT_EQ(10, e4.error());
}

TEST(Expected, testAssignableFromMovedValue) {
  std::string world = "world";
  exp_string e = "hello";
  e = std::move(world);

  EXPECT_TRUE(e.has_value());
  EXPECT_EQ("world", e.value());
  EXPECT_EQ("", world);
}

TEST(Expected, testAssignableFromMovedUnexpected) {
  std::string world = "world";
  exp_string e = "hello";
  e = unexpected(std::move(world));

  EXPECT_FALSE(e.has_value());
  EXPECT_EQ("world", e.error());
  EXPECT_EQ("", world);
}

TEST(Expected, testEmplace) {
  struct T {
    int a;
    double b;
    T() {}
    T(int a, double b) noexcept : a(a), b(b) {}
  };
  expected<T, int> exp;
  T& t = exp.emplace(3, 10.5f);

  EXPECT_TRUE(exp.has_value());
  EXPECT_EQ(3, t.a);
  EXPECT_EQ(10.5f, t.b);
  EXPECT_EQ(3, exp.value().a);
  EXPECT_EQ(10.5, exp.value().b);

  exp_void e = unexpected(10);
  ASSERT_FALSE(e.has_value());
  e.emplace();
  EXPECT_TRUE(e.has_value());
}

TEST(Expected, testSwapExpectedExpected) {
  exp_int e = 10;
  exp_int e2 = 20;
  e.swap(e2);

  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(20, e.value());
  EXPECT_EQ(10, e2.value());

  exp_void e3;
  exp_void e4;
  e3.swap(e4);

  EXPECT_TRUE(e3.has_value());
  EXPECT_TRUE(e4.has_value());
}

TEST(Expected, testSwapUnexpectedUnexpected) {
  exp_int e = unexpected(10);
  exp_int e2 = unexpected(20);
  e.swap(e2);
  EXPECT_FALSE(e.has_value());
  EXPECT_FALSE(e2.has_value());
  EXPECT_EQ(20, e.error());
  EXPECT_EQ(10, e2.error());

  exp_void e3 = unexpected(10);
  exp_void e4 = unexpected(20);
  e3.swap(e4);
  EXPECT_FALSE(e3.has_value());
  EXPECT_FALSE(e4.has_value());
  EXPECT_EQ(20, e3.error());
  EXPECT_EQ(10, e4.error());
}

TEST(Expected, testSwapExpectedUnepected) {
  exp_int e = 10;
  exp_int e2 = unexpected(30);
  e.swap(e2);
  EXPECT_FALSE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(30, e.error());
  EXPECT_EQ(10, e2.value());

  exp_void e3;
  exp_void e4 = unexpected(10);
  e3.swap(e4);
  EXPECT_FALSE(e3.has_value());
  EXPECT_TRUE(e4.has_value());
  EXPECT_EQ(10, e3.error());
}

TEST(Expected, testDereference) {
  struct T {
    int a;
    double b;
    T() {}
    T(int a, double b) : a(a), b(b) {}
  };
  expected<T, int> exp = T(3, 10.5f);

  EXPECT_EQ(3, exp->a);
  EXPECT_EQ(10.5f, exp->b);

  EXPECT_EQ(3, (*exp).a);
  EXPECT_EQ(10.5f, (*exp).b);
}

TEST(Expected, testTest) {
  exp_int e = 10;
  EXPECT_TRUE(e.ok());
  EXPECT_TRUE(e.has_value());

  exp_int e2 = unexpected(10);
  EXPECT_FALSE(e2.ok());
  EXPECT_FALSE(e2.has_value());
}

TEST(Expected, testGetValue) {
  exp_int e = 10;
  EXPECT_EQ(10, e.value());
  EXPECT_EQ(10, e.value_or(20));

  exp_int e2 = unexpected(10);
  EXPECT_EQ(10, e2.error());
  EXPECT_EQ(20, e2.value_or(20));
}

TEST(Expected, testSameValues) {
  exp_int e = 10;
  exp_int e2 = 10;
  EXPECT_TRUE(e == e2);
  EXPECT_TRUE(e2 == e);
  EXPECT_FALSE(e != e2);
  EXPECT_FALSE(e2 != e);

  exp_void e3;
  exp_void e4;
  EXPECT_TRUE(e3 == e4);
  EXPECT_TRUE(e4 == e3);
  EXPECT_FALSE(e3 != e4);
  EXPECT_FALSE(e4 != e3);
}

TEST(Expected, testDifferentValues) {
  exp_int e = 10;
  exp_int e2 = 20;
  EXPECT_FALSE(e == e2);
  EXPECT_FALSE(e2 == e);
  EXPECT_TRUE(e != e2);
  EXPECT_TRUE(e2 != e);
}

TEST(Expected, testValueWithError) {
  exp_int e = 10;
  exp_int e2 = unexpected(10);
  EXPECT_FALSE(e == e2);
  EXPECT_FALSE(e2 == e);
  EXPECT_TRUE(e != e2);
  EXPECT_TRUE(e2 != e);

  exp_void e3;
  exp_void e4 = unexpected(10);
  EXPECT_FALSE(e3 == e4);
  EXPECT_FALSE(e4 == e3);
  EXPECT_TRUE(e3 != e4);
  EXPECT_TRUE(e4 != e3);
}

TEST(Expected, testSameErrors) {
  exp_int e = unexpected(10);
  exp_int e2 = unexpected(10);
  EXPECT_TRUE(e == e2);
  EXPECT_TRUE(e2 == e);
  EXPECT_FALSE(e != e2);
  EXPECT_FALSE(e2 != e);

  exp_void e3 = unexpected(10);
  exp_void e4 = unexpected(10);
  EXPECT_TRUE(e3 == e4);
  EXPECT_TRUE(e4 == e3);
  EXPECT_FALSE(e3 != e4);
  EXPECT_FALSE(e4 != e3);
}

TEST(Expected, testDifferentErrors) {
  exp_int e = unexpected(10);
  exp_int e2 = unexpected(20);
  EXPECT_FALSE(e == e2);
  EXPECT_FALSE(e2 == e);
  EXPECT_TRUE(e != e2);
  EXPECT_TRUE(e2 != e);

  exp_void e3 = unexpected(10);
  exp_void e4 = unexpected(20);
  EXPECT_FALSE(e3 == e4);
  EXPECT_FALSE(e4 == e3);
  EXPECT_TRUE(e3 != e4);
  EXPECT_TRUE(e4 != e3);
}

TEST(Expected, testCompareWithSameError) {
  exp_int e = unexpected(10);
  exp_int::unexpected_type error = 10;
  EXPECT_TRUE(e == error);
  EXPECT_TRUE(error == e);
  EXPECT_FALSE(e != error);
  EXPECT_FALSE(error != e);

  exp_void e2 = unexpected(10);
  exp_void::unexpected_type error2 = 10;
  EXPECT_TRUE(e2 == error2);
  EXPECT_TRUE(error2 == e2);
  EXPECT_FALSE(e2 != error2);
  EXPECT_FALSE(error2 != e2);
}

TEST(Expected, testCompareWithDifferentError) {
  exp_int e = unexpected(10);
  exp_int::unexpected_type error = 20;
  EXPECT_FALSE(e == error);
  EXPECT_FALSE(error == e);
  EXPECT_TRUE(e != error);
  EXPECT_TRUE(error != e);

  exp_void e2 = unexpected(10);
  exp_void::unexpected_type error2 = 20;
  EXPECT_FALSE(e2 == error2);
  EXPECT_FALSE(error2 == e2);
  EXPECT_TRUE(e2 != error2);
  EXPECT_TRUE(error2 != e2);
}

TEST(Expected, testCompareDifferentType) {
  expected<int,int> e = 10;
  expected<int32_t, int> e2 = 10;
  EXPECT_TRUE(e == e2);
  e2 = 20;
  EXPECT_FALSE(e == e2);

  expected<std::string_view,int> e3 = "hello";
  expected<std::string,int> e4 = "hello";
  EXPECT_TRUE(e3 == e4);
  e4 = "world";
  EXPECT_FALSE(e3 == e4);

  expected<void,int> e5;
  expected<int,int> e6 = 10;
  EXPECT_FALSE(e5 == e6);
  EXPECT_FALSE(e6 == e5);
}

TEST(Expected, testDivideExample) {
  struct QR {
    int quotient;
    int remainder;
    QR(int q, int r) noexcept : quotient(q), remainder(r) {}
    bool operator==(const QR& rhs) const {
      return quotient == rhs.quotient && remainder == rhs.remainder;
    }
    bool operator!=(const QR& rhs) const {
      return quotient != rhs.quotient || remainder == rhs.remainder;
    }
  };

  auto divide = [](int x, int y) -> expected<QR,E> {
    if (y == 0) {
      return unexpected(E("divide by zero", -1));
    } else {
      return QR(x / y, x % y);
    }
  };

  EXPECT_FALSE(divide(10, 0).ok());
  EXPECT_EQ("divide by zero", divide(10, 0).error().message);
  EXPECT_EQ(-1, divide(10, 0).error().cause);

  EXPECT_TRUE(divide(10, 3).ok());
  EXPECT_EQ(QR(3, 1), *divide(10, 3));
}

TEST(Expected, testPair) {
  auto test = [](bool yes) -> exp_pair {
    if (yes) {
      return exp_pair({"yes", 42});
    } else {
      return unexpected(42);
    }
  };

  auto r = test(true);
  EXPECT_TRUE(r.ok());
  EXPECT_EQ("yes", r->first);
}

TEST(Expected, testVoid) {
  auto test = [](bool ok) -> exp_void {
    if (ok) {
      return {};
    } else {
      return unexpected(10);
    }
  };

  auto r = test(true);
  EXPECT_TRUE(r.ok());
  r = test(false);
  EXPECT_FALSE(r.ok());
  EXPECT_EQ(10, r.error());
}

// copied from result_test.cpp
struct ConstructorTracker {
  static size_t constructor_called;
  static size_t copy_constructor_called;
  static size_t move_constructor_called;
  static size_t copy_assignment_called;
  static size_t move_assignment_called;

  template <typename T,
    typename std::enable_if_t<std::is_convertible_v<T, std::string>>* = nullptr>
  ConstructorTracker(T&& string) : string(string) {
    ++constructor_called;
  }
  ConstructorTracker(const ConstructorTracker& ct) {
    ++copy_constructor_called;
    string = ct.string;
  }
  ConstructorTracker(ConstructorTracker&& ct) noexcept {
    ++move_constructor_called;
    string = std::move(ct.string);
  }
  ConstructorTracker& operator=(const ConstructorTracker& ct) {
    ++copy_assignment_called;
    string = ct.string;
    return *this;
  }
  ConstructorTracker& operator=(ConstructorTracker&& ct) noexcept {
    ++move_assignment_called;
    string = std::move(ct.string);
    return *this;
  }
  static void Reset() {
    constructor_called = 0;
    copy_constructor_called = 0;
    move_constructor_called = 0;
    copy_assignment_called = 0;
    move_assignment_called = 0;
  }
  std::string string;
};

size_t ConstructorTracker::constructor_called = 0;
size_t ConstructorTracker::copy_constructor_called = 0;
size_t ConstructorTracker::move_constructor_called = 0;
size_t ConstructorTracker::copy_assignment_called = 0;
size_t ConstructorTracker::move_assignment_called = 0;

typedef expected<ConstructorTracker, int> exp_track;

TEST(Expected, testNumberOfCopies) {
  // default constructor
  ConstructorTracker::Reset();
  exp_track e("hello");
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // copy constructor
  ConstructorTracker::Reset();
  exp_track e2 = e;
  EXPECT_EQ(0U, ConstructorTracker::constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // move constructor
  ConstructorTracker::Reset();
  exp_track e3 = std::move(e);
  EXPECT_EQ(0U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // construct from lvalue
  ConstructorTracker::Reset();
  ConstructorTracker ct = "hello";
  exp_track e4(ct);
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // construct from rvalue
  ConstructorTracker::Reset();
  ConstructorTracker ct2 = "hello";
  exp_track e5(std::move(ct2));
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // copy assignment
  ConstructorTracker::Reset();
  exp_track e6 = "hello";
  exp_track e7 = "world";
  e7 = e6;
  EXPECT_EQ(2U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  // move assignment
  ConstructorTracker::Reset();
  exp_track e8 = "hello";
  exp_track e9 = "world";
  e9 = std::move(e8);
  EXPECT_EQ(2U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(1U, ConstructorTracker::move_assignment_called);

  // swap
  ConstructorTracker::Reset();
  exp_track e10 = "hello";
  exp_track e11 = "world";
  std::swap(e10, e11);
  EXPECT_EQ(2U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(2U, ConstructorTracker::move_assignment_called);
}

TEST(Expected, testNoCopyOnReturn) {
  auto test = [](const std::string& in) -> exp_track {
    if (in.empty()) {
      return "literal string";
    }
    if (in == "test2") {
      return ConstructorTracker(in + in + "2");
    }
    ConstructorTracker result(in + " " + in);
    return result;
  };

  ConstructorTracker::Reset();
  auto result1 = test("");
  ASSERT_TRUE(result1.ok());
  EXPECT_EQ("literal string", result1->string);
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  ConstructorTracker::Reset();
  auto result2 = test("test2");
  ASSERT_TRUE(result2.ok());
  EXPECT_EQ("test2test22", result2->string);
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  ConstructorTracker::Reset();
  auto result3 = test("test3");
  ASSERT_TRUE(result3.ok());
  EXPECT_EQ("test3 test3", result3->string);
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);
}

TEST(Expected, testNested) {
  expected<exp_string, std::string> e = "hello";

  EXPECT_TRUE(e.ok());
  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e.value().has_value());
  EXPECT_TRUE(e->ok());
  EXPECT_EQ("hello", e.value().value());

  expected<exp_string, std::string> e2 = unexpected("world");
  EXPECT_FALSE(e2.has_value());
  EXPECT_FALSE(e2.ok());
  EXPECT_EQ("world", e2.error());

  expected<exp_string, std::string> e3 = exp_string(unexpected("world"));
  EXPECT_TRUE(e3.has_value());
  EXPECT_FALSE(e3.value().has_value());
  EXPECT_TRUE(e3.ok());
  EXPECT_FALSE(e3->ok());
  EXPECT_EQ("world", e3.value().error());
}

constexpr bool equals(const char* a, const char* b) {
  return (a == nullptr && b == nullptr) ||
      (a != nullptr && b != nullptr && *a == *b &&
       (*a == '\0' || equals(a + 1, b + 1)));
}

TEST(Expected, testConstexpr) {
  // Compliation error will occur if these expressions can't be
  // evaluated at compile time
  constexpr exp_int e(3);
  constexpr exp_int::unexpected_type err(3);
  constexpr int i = 4;

  // default constructor
  static_assert(exp_int().value() == 0);
  // copy constructor
  static_assert(exp_int(e).value() == 3);
  // move constructor
  static_assert(exp_int(exp_int(4)).value() == 4);
  // copy construct from value
  static_assert(exp_int(i).value() == 4);
  // copy construct from unexpected
  static_assert(exp_int(err).error() == 3);
  // move costruct from unexpected
  static_assert(exp_int(unexpected(3)).error() == 3);
  // observers
  static_assert(*exp_int(3) == 3);
  static_assert(exp_int(3).has_value() == true);
  static_assert(exp_int(3).value_or(4) == 3);

  typedef expected<const char*, int> exp_s;
  constexpr exp_s s("hello");
  constexpr const char* c = "hello";
  static_assert(equals(exp_s().value(), nullptr));
  static_assert(equals(exp_s(s).value(), "hello"));
  static_assert(equals(exp_s(exp_s("hello")).value(), "hello"));
  static_assert(equals(exp_s("hello").value(), "hello"));
  static_assert(equals(exp_s(c).value(), "hello"));
}

TEST(Expected, testWithNonConstructible) {
   struct AssertNotConstructed {
     AssertNotConstructed() = delete;
   };

   expected<int, AssertNotConstructed> v(42);
   EXPECT_TRUE(v.has_value());
   EXPECT_EQ(42, v.value());

   expected<AssertNotConstructed, int> e(unexpected(42));
   EXPECT_FALSE(e.has_value());
   EXPECT_EQ(42, e.error());
}

TEST(Expected, testWithMoveOnlyType) {
  typedef expected<std::unique_ptr<int>,std::unique_ptr<int>> exp_ptr;
  exp_ptr e(std::make_unique<int>(3));
  exp_ptr e2(unexpected(std::make_unique<int>(4)));

  EXPECT_TRUE(e.has_value());
  EXPECT_FALSE(e2.has_value());
  EXPECT_EQ(3, *(e.value()));
  EXPECT_EQ(4, *(e2.error()));

  e2 = std::move(e);
  EXPECT_TRUE(e.has_value());
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(3, *(e2.value()));
}
