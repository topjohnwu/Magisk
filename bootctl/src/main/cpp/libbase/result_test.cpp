/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "android-base/result.h"

#include "errno.h"

#include <istream>
#include <string>

#include <gtest/gtest.h>

using namespace std::string_literals;

namespace android {
namespace base {

TEST(result, result_accessors) {
  Result<std::string> result = "success";
  ASSERT_RESULT_OK(result);
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ("success", *result);
  EXPECT_EQ("success", result.value());

  EXPECT_EQ('s', result->data()[0]);
}

TEST(result, result_accessors_rvalue) {
  ASSERT_TRUE(Result<std::string>("success").ok());
  ASSERT_TRUE(Result<std::string>("success").has_value());

  EXPECT_EQ("success", *Result<std::string>("success"));
  EXPECT_EQ("success", Result<std::string>("success").value());

  EXPECT_EQ('s', Result<std::string>("success")->data()[0]);
}

TEST(result, result_void) {
  Result<void> ok = {};
  EXPECT_RESULT_OK(ok);
  ok.value();  // should not crash
  ASSERT_DEATH(ok.error(), "");

  Result<void> fail = Error() << "failure" << 1;
  EXPECT_FALSE(fail.ok());
  EXPECT_EQ("failure1", fail.error().message());
  EXPECT_EQ(0, fail.error().code());
  EXPECT_TRUE(ok != fail);
  ASSERT_DEATH(fail.value(), "");

  auto test = [](bool ok) -> Result<void> {
    if (ok) return {};
    else return Error() << "failure" << 1;
  };
  EXPECT_TRUE(test(true).ok());
  EXPECT_FALSE(test(false).ok());
  test(true).value();  // should not crash
  ASSERT_DEATH(test(true).error(), "");
  ASSERT_DEATH(test(false).value(), "");
  EXPECT_EQ("failure1", test(false).error().message());
}

TEST(result, result_error) {
  Result<void> result = Error() << "failure" << 1;
  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  EXPECT_EQ(0, result.error().code());
  EXPECT_EQ("failure1", result.error().message());
}

TEST(result, result_error_empty) {
  Result<void> result = Error();
  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  EXPECT_EQ(0, result.error().code());
  EXPECT_EQ("", result.error().message());
}

TEST(result, result_error_rvalue) {
  // Error() and ErrnoError() aren't actually used to create a Result<T> object.
  // Under the hood, they are an intermediate class that can be implicitly constructed into a
  // Result<T>.  This is needed both to create the ostream and because Error() itself, by
  // definition will not know what the type, T, of the underlying Result<T> object that it would
  // create is.

  auto MakeRvalueErrorResult = []() -> Result<void> { return Error() << "failure" << 1; };
  ASSERT_FALSE(MakeRvalueErrorResult().ok());
  ASSERT_FALSE(MakeRvalueErrorResult().has_value());

  EXPECT_EQ(0, MakeRvalueErrorResult().error().code());
  EXPECT_EQ("failure1", MakeRvalueErrorResult().error().message());
}

TEST(result, result_errno_error) {
  constexpr int test_errno = 6;
  errno = test_errno;
  Result<void> result = ErrnoError() << "failure" << 1;

  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  EXPECT_EQ(test_errno, result.error().code());
  EXPECT_EQ("failure1: "s + strerror(test_errno), result.error().message());
}

TEST(result, result_errno_error_no_text) {
  constexpr int test_errno = 6;
  errno = test_errno;
  Result<void> result = ErrnoError();

  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  EXPECT_EQ(test_errno, result.error().code());
  EXPECT_EQ(strerror(test_errno), result.error().message());
}

TEST(result, result_error_from_other_result) {
  auto error_text = "test error"s;
  Result<void> result = Error() << error_text;

  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  Result<std::string> result2 = result.error();

  ASSERT_FALSE(result2.ok());
  ASSERT_FALSE(result2.has_value());

  EXPECT_EQ(0, result2.error().code());
  EXPECT_EQ(error_text, result2.error().message());
}

TEST(result, result_error_through_ostream) {
  auto error_text = "test error"s;
  Result<void> result = Error() << error_text;

  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  Result<std::string> result2 = Error() << result.error();

  ASSERT_FALSE(result2.ok());
  ASSERT_FALSE(result2.has_value());

  EXPECT_EQ(0, result2.error().code());
  EXPECT_EQ(error_text, result2.error().message());
}

TEST(result, result_errno_error_through_ostream) {
  auto error_text = "test error"s;
  constexpr int test_errno = 6;
  errno = 6;
  Result<void> result = ErrnoError() << error_text;

  errno = 0;

  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(result.has_value());

  Result<std::string> result2 = Error() << result.error();

  ASSERT_FALSE(result2.ok());
  ASSERT_FALSE(result2.has_value());

  EXPECT_EQ(test_errno, result2.error().code());
  EXPECT_EQ(error_text + ": " + strerror(test_errno), result2.error().message());
}

TEST(result, constructor_forwarding) {
  auto result = Result<std::string>(std::in_place, 5, 'a');

  ASSERT_RESULT_OK(result);
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ("aaaaa", *result);
}

struct ConstructorTracker {
  static size_t constructor_called;
  static size_t copy_constructor_called;
  static size_t move_constructor_called;
  static size_t copy_assignment_called;
  static size_t move_assignment_called;

  template <typename T>
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

  std::string string;
};

size_t ConstructorTracker::constructor_called = 0;
size_t ConstructorTracker::copy_constructor_called = 0;
size_t ConstructorTracker::move_constructor_called = 0;
size_t ConstructorTracker::copy_assignment_called = 0;
size_t ConstructorTracker::move_assignment_called = 0;

Result<ConstructorTracker> ReturnConstructorTracker(const std::string& in) {
  if (in.empty()) {
    return "literal string";
  }
  if (in == "test2") {
    return ConstructorTracker(in + in + "2");
  }
  ConstructorTracker result(in + " " + in);
  return result;
};

TEST(result, no_copy_on_return) {
  // If returning parameters that may be used to implicitly construct the type T of Result<T>,
  // then those parameters are forwarded to the construction of Result<T>.

  // If returning an prvalue or xvalue, it will be move constructed during the construction of
  // Result<T>.

  // This check ensures that that is the case, and particularly that no copy constructors
  // are called.

  auto result1 = ReturnConstructorTracker("");
  ASSERT_RESULT_OK(result1);
  EXPECT_EQ("literal string", result1->string);
  EXPECT_EQ(1U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  auto result2 = ReturnConstructorTracker("test2");
  ASSERT_RESULT_OK(result2);
  EXPECT_EQ("test2test22", result2->string);
  EXPECT_EQ(2U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(1U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);

  auto result3 = ReturnConstructorTracker("test3");
  ASSERT_RESULT_OK(result3);
  EXPECT_EQ("test3 test3", result3->string);
  EXPECT_EQ(3U, ConstructorTracker::constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_constructor_called);
  EXPECT_EQ(2U, ConstructorTracker::move_constructor_called);
  EXPECT_EQ(0U, ConstructorTracker::copy_assignment_called);
  EXPECT_EQ(0U, ConstructorTracker::move_assignment_called);
}

// Below two tests require that we do not hide the move constructor with our forwarding reference
// constructor.  This is done with by disabling the forwarding reference constructor if its first
// and only type is Result<T>.
TEST(result, result_result_with_success) {
  auto return_result_result_with_success = []() -> Result<Result<void>> { return Result<void>(); };
  auto result = return_result_result_with_success();
  ASSERT_RESULT_OK(result);
  ASSERT_RESULT_OK(*result);

  auto inner_result = result.value();
  ASSERT_RESULT_OK(inner_result);
}

TEST(result, result_result_with_failure) {
  auto return_result_result_with_error = []() -> Result<Result<void>> {
    return Result<void>(ResultError("failure string", 6));
  };
  auto result = return_result_result_with_error();
  ASSERT_RESULT_OK(result);
  ASSERT_FALSE(result->ok());
  EXPECT_EQ("failure string", (*result).error().message());
  EXPECT_EQ(6, (*result).error().code());
}

// This test requires that we disable the forwarding reference constructor if Result<T> is the
// *only* type that we are forwarding.  In otherwords, if we are forwarding Result<T>, int to
// construct a Result<T>, then we still need the constructor.
TEST(result, result_two_parameter_constructor_same_type) {
  struct TestStruct {
    TestStruct(int value) : value_(value) {}
    TestStruct(Result<TestStruct> result, int value) : value_(result->value_ * value) {}
    int value_;
  };

  auto return_test_struct = []() -> Result<TestStruct> {
    return Result<TestStruct>(std::in_place, Result<TestStruct>(std::in_place, 6), 6);
  };

  auto result = return_test_struct();
  ASSERT_RESULT_OK(result);
  EXPECT_EQ(36, result->value_);
}

TEST(result, die_on_access_failed_result) {
  Result<std::string> result = Error();
  ASSERT_DEATH(*result, "");
}

TEST(result, die_on_get_error_succesful_result) {
  Result<std::string> result = "success";
  ASSERT_DEATH(result.error(), "");
}

template <class CharT>
std::basic_ostream<CharT>& SetErrnoToTwo(std::basic_ostream<CharT>& ss) {
  errno = 2;
  return ss;
}

TEST(result, preserve_errno) {
  errno = 1;
  int old_errno = errno;
  Result<int> result = Error() << "Failed" << SetErrnoToTwo<char>;
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(old_errno, errno);

  errno = 1;
  old_errno = errno;
  Result<int> result2 = ErrnoError() << "Failed" << SetErrnoToTwo<char>;
  ASSERT_FALSE(result2.ok());
  EXPECT_EQ(old_errno, errno);
  EXPECT_EQ(old_errno, result2.error().code());
}

TEST(result, error_with_fmt) {
  Result<int> result = Errorf("{} {}!", "hello", "world");
  EXPECT_EQ("hello world!", result.error().message());

  result = Errorf("{} {}!", std::string("hello"), std::string("world"));
  EXPECT_EQ("hello world!", result.error().message());

  result = Errorf("{1} {0}!", "world", "hello");
  EXPECT_EQ("hello world!", result.error().message());

  result = Errorf("hello world!");
  EXPECT_EQ("hello world!", result.error().message());

  Result<int> result2 = Errorf("error occurred with {}", result.error());
  EXPECT_EQ("error occurred with hello world!", result2.error().message());

  constexpr int test_errno = 6;
  errno = test_errno;
  result = ErrnoErrorf("{} {}!", "hello", "world");
  EXPECT_EQ(test_errno, result.error().code());
  EXPECT_EQ("hello world!: "s + strerror(test_errno), result.error().message());
}

TEST(result, error_with_fmt_carries_errno) {
  constexpr int inner_errno = 6;
  errno = inner_errno;
  Result<int> inner_result = ErrnoErrorf("inner failure");
  errno = 0;
  EXPECT_EQ(inner_errno, inner_result.error().code());

  // outer_result is created with Errorf, but its error code is got from inner_result.
  Result<int> outer_result = Errorf("outer failure caused by {}", inner_result.error());
  EXPECT_EQ(inner_errno, outer_result.error().code());
  EXPECT_EQ("outer failure caused by inner failure: "s + strerror(inner_errno),
            outer_result.error().message());

  // now both result objects are created with ErrnoErrorf. errno from the inner_result
  // is not passed to outer_result.
  constexpr int outer_errno = 10;
  errno = outer_errno;
  outer_result = ErrnoErrorf("outer failure caused by {}", inner_result.error());
  EXPECT_EQ(outer_errno, outer_result.error().code());
  EXPECT_EQ("outer failure caused by inner failure: "s + strerror(inner_errno) + ": "s +
                strerror(outer_errno),
            outer_result.error().message());
}

TEST(result, errno_chaining_multiple) {
  constexpr int errno1 = 6;
  errno = errno1;
  Result<int> inner1 = ErrnoErrorf("error1");

  constexpr int errno2 = 10;
  errno = errno2;
  Result<int> inner2 = ErrnoErrorf("error2");

  // takes the error code of inner2 since its the last one.
  Result<int> outer = Errorf("two errors: {}, {}", inner1.error(), inner2.error());
  EXPECT_EQ(errno2, outer.error().code());
  EXPECT_EQ("two errors: error1: "s + strerror(errno1) + ", error2: "s + strerror(errno2),
            outer.error().message());
}

}  // namespace base
}  // namespace android
