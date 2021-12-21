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

// This file contains classes for returning a successful result along with an optional
// arbitrarily typed return value or for returning a failure result along with an optional string
// indicating why the function failed.

// There are 3 classes that implement this functionality and one additional helper type.
//
// Result<T> either contains a member of type T that can be accessed using similar semantics as
// std::optional<T> or it contains a ResultError describing an error, which can be accessed via
// Result<T>::error().
//
// ResultError is a type that contains both a std::string describing the error and a copy of errno
// from when the error occurred.  ResultError can be used in an ostream directly to print its
// string value.
//
// Result<void> is the correct return type for a function that either returns successfully or
// returns an error value.  Returning {} from a function that returns Result<void> is the
// correct way to indicate that a function without a return type has completed successfully.
//
// A successful Result<T> is constructed implicitly from any type that can be implicitly converted
// to T or from the constructor arguments for T.  This allows you to return a type T directly from
// a function that returns Result<T>.
//
// Error and ErrnoError are used to construct a Result<T> that has failed.  The Error class takes
// an ostream as an input and are implicitly cast to a Result<T> containing that failure.
// ErrnoError() is a helper function to create an Error class that appends ": " + strerror(errno)
// to the end of the failure string to aid in interacting with C APIs.  Alternatively, an errno
// value can be directly specified via the Error() constructor.
//
// Errorf and ErrnoErrorf accept the format string syntax of the fmblib (https://fmt.dev).
// Errorf("{} errors", num) is equivalent to Error() << num << " errors".
//
// ResultError can be used in the ostream and when using Error/Errorf to construct a Result<T>.
// In this case, the string that the ResultError takes is passed through the stream normally, but
// the errno is passed to the Result<T>. This can be used to pass errno from a failing C function up
// multiple callers. Note that when the outer Result<T> is created with ErrnoError/ErrnoErrorf then
// the errno from the inner ResultError is not passed. Also when multiple ResultError objects are
// used, the errno of the last one is respected.
//
// ResultError can also directly construct a Result<T>.  This is particularly useful if you have a
// function that return Result<T> but you have a Result<U> and want to return its error.  In this
// case, you can return the .error() from the Result<U> to construct the Result<T>.

// An example of how to use these is below:
// Result<U> CalculateResult(const T& input) {
//   U output;
//   if (!SomeOtherCppFunction(input, &output)) {
//     return Errorf("SomeOtherCppFunction {} failed", input);
//   }
//   if (!c_api_function(output)) {
//     return ErrnoErrorf("c_api_function {} failed", output);
//   }
//   return output;
// }
//
// auto output = CalculateResult(input);
// if (!output) return Error() << "CalculateResult failed: " << output.error();
// UseOutput(*output);

#pragma once

#include <errno.h>

#include <sstream>
#include <string>

#include "android-base/expected.h"
#include "android-base/format.h"

namespace android {
namespace base {

struct ResultError {
  template <typename T>
  ResultError(T&& message, int code) : message_(std::forward<T>(message)), code_(code) {}

  template <typename T>
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator android::base::expected<T, ResultError>() {
    return android::base::unexpected(ResultError(message_, code_));
  }

  std::string message() const { return message_; }
  int code() const { return code_; }

 private:
  std::string message_;
  int code_;
};

inline bool operator==(const ResultError& lhs, const ResultError& rhs) {
  return lhs.message() == rhs.message() && lhs.code() == rhs.code();
}

inline bool operator!=(const ResultError& lhs, const ResultError& rhs) {
  return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const ResultError& t) {
  os << t.message();
  return os;
}

class Error {
 public:
  Error() : errno_(0), append_errno_(false) {}
  // NOLINTNEXTLINE(google-explicit-constructor)
  Error(int errno_to_append) : errno_(errno_to_append), append_errno_(true) {}

  template <typename T>
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator android::base::expected<T, ResultError>() {
    return android::base::unexpected(ResultError(str(), errno_));
  }

  template <typename T>
  Error& operator<<(T&& t) {
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon)
    if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, ResultError>) {
      errno_ = t.code();
      return (*this) << t.message();
    }
    int saved = errno;
    ss_ << t;
    errno = saved;
    return *this;
  }

  const std::string str() const {
    std::string str = ss_.str();
    if (append_errno_) {
      if (str.empty()) {
        return strerror(errno_);
      }
      return std::move(str) + ": " + strerror(errno_);
    }
    return str;
  }

  Error(const Error&) = delete;
  Error(Error&&) = delete;
  Error& operator=(const Error&) = delete;
  Error& operator=(Error&&) = delete;

  template <typename T, typename... Args>
  friend Error ErrorfImpl(const T&& fmt, const Args&... args);

  template <typename T, typename... Args>
  friend Error ErrnoErrorfImpl(const T&& fmt, const Args&... args);

 private:
  Error(bool append_errno, int errno_to_append, const std::string& message)
      : errno_(errno_to_append), append_errno_(append_errno) {
    (*this) << message;
  }

  std::stringstream ss_;
  int errno_;
  const bool append_errno_;
};

inline Error ErrnoError() {
  return Error(errno);
}

inline int ErrorCode(int code) {
  return code;
}

// Return the error code of the last ResultError object, if any.
// Otherwise, return `code` as it is.
template <typename T, typename... Args>
inline int ErrorCode(int code, T&& t, const Args&... args) {
  if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, ResultError>) {
    return ErrorCode(t.code(), args...);
  }
  return ErrorCode(code, args...);
}

template <typename T, typename... Args>
inline Error ErrorfImpl(const T&& fmt, const Args&... args) {
  return Error(false, ErrorCode(0, args...), fmt::format(fmt, args...));
}

template <typename T, typename... Args>
inline Error ErrnoErrorfImpl(const T&& fmt, const Args&... args) {
  return Error(true, errno, fmt::format(fmt, args...));
}

#define Errorf(fmt, ...) android::base::ErrorfImpl(FMT_STRING(fmt), ##__VA_ARGS__)
#define ErrnoErrorf(fmt, ...) android::base::ErrnoErrorfImpl(FMT_STRING(fmt), ##__VA_ARGS__)

template <typename T>
using Result = android::base::expected<T, ResultError>;

// Macros for testing the results of functions that return android::base::Result.
// These also work with base::android::expected.

#define CHECK_RESULT_OK(stmt)       \
  do {                              \
    const auto& tmp = (stmt);       \
    CHECK(tmp.ok()) << tmp.error(); \
  } while (0)

#define ASSERT_RESULT_OK(stmt)            \
  do {                                    \
    const auto& tmp = (stmt);             \
    ASSERT_TRUE(tmp.ok()) << tmp.error(); \
  } while (0)

#define EXPECT_RESULT_OK(stmt)            \
  do {                                    \
    auto tmp = (stmt);                    \
    EXPECT_TRUE(tmp.ok()) << tmp.error(); \
  } while (0)

// TODO: Maybe add RETURN_IF_ERROR() and ASSIGN_OR_RETURN()

}  // namespace base
}  // namespace android
