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

#pragma once

#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <variant>

// android::base::expected is an Android implementation of the std::expected
// proposal.
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0323r7.html
//
// Usage:
// using android::base::expected;
// using android::base::unexpected;
//
// expected<double,std::string> safe_divide(double i, double j) {
//   if (j == 0) return unexpected("divide by zero");
//   else return i / j;
// }
//
// void test() {
//   auto q = safe_divide(10, 0);
//   if (q) { printf("%f\n", q.value()); }
//   else { printf("%s\n", q.error().c_str()); }
// }
//
// When the proposal becomes part of the standard and is implemented by
// libcxx, this will be removed and android::base::expected will be
// type alias to std::expected.
//

namespace android {
namespace base {

// Synopsis
template<class T, class E>
    class expected;

template<class E>
    class unexpected;
template<class E>
  unexpected(E) -> unexpected<E>;

template<class E>
   class bad_expected_access;

template<>
   class bad_expected_access<void>;

struct unexpect_t {
   explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};

// macros for SFINAE
#define _ENABLE_IF(...) \
  , std::enable_if_t<(__VA_ARGS__)>* = nullptr

// Define NODISCARD_EXPECTED to prevent expected<T,E> from being
// ignored when used as a return value. This is off by default.
#ifdef NODISCARD_EXPECTED
#define _NODISCARD_ [[nodiscard]]
#else
#define _NODISCARD_
#endif

// Class expected
template<class T, class E>
class _NODISCARD_ expected {
 public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template<class U>
  using rebind = expected<U, error_type>;

  // constructors
  constexpr expected() = default;
  constexpr expected(const expected& rhs) = default;
  constexpr expected(expected&& rhs) noexcept = default;

  template<class U, class G _ENABLE_IF(
    std::is_constructible_v<T, const U&> &&
    std::is_constructible_v<E, const G&> &&
    !std::is_constructible_v<T, expected<U, G>&> &&
    !std::is_constructible_v<T, expected<U, G>&&> &&
    !std::is_constructible_v<T, const expected<U, G>&> &&
    !std::is_constructible_v<T, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, T> &&
    !std::is_convertible_v<expected<U, G>&&, T> &&
    !std::is_convertible_v<const expected<U, G>&, T> &&
    !std::is_convertible_v<const expected<U, G>&&, T> &&
    !(!std::is_convertible_v<const U&, T> ||
     !std::is_convertible_v<const G&, E>) /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(const expected<U, G>& rhs) {
    if (rhs.has_value()) var_ = rhs.value();
    else var_ = unexpected(rhs.error());
  }

  template<class U, class G _ENABLE_IF(
    std::is_constructible_v<T, const U&> &&
    std::is_constructible_v<E, const G&> &&
    !std::is_constructible_v<T, expected<U, G>&> &&
    !std::is_constructible_v<T, expected<U, G>&&> &&
    !std::is_constructible_v<T, const expected<U, G>&> &&
    !std::is_constructible_v<T, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, T> &&
    !std::is_convertible_v<expected<U, G>&&, T> &&
    !std::is_convertible_v<const expected<U, G>&, T> &&
    !std::is_convertible_v<const expected<U, G>&&, T> &&
    (!std::is_convertible_v<const U&, T> ||
     !std::is_convertible_v<const G&, E>) /* explicit */
  )>
  constexpr explicit expected(const expected<U, G>& rhs) {
    if (rhs.has_value()) var_ = rhs.value();
    else var_ = unexpected(rhs.error());
  }

  template<class U, class G _ENABLE_IF(
    std::is_constructible_v<T, const U&> &&
    std::is_constructible_v<E, const G&> &&
    !std::is_constructible_v<T, expected<U, G>&> &&
    !std::is_constructible_v<T, expected<U, G>&&> &&
    !std::is_constructible_v<T, const expected<U, G>&> &&
    !std::is_constructible_v<T, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, T> &&
    !std::is_convertible_v<expected<U, G>&&, T> &&
    !std::is_convertible_v<const expected<U, G>&, T> &&
    !std::is_convertible_v<const expected<U, G>&&, T> &&
    !(!std::is_convertible_v<const U&, T> ||
     !std::is_convertible_v<const G&, E>) /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(expected<U, G>&& rhs) {
    if (rhs.has_value()) var_ = std::move(rhs.value());
    else var_ = unexpected(std::move(rhs.error()));
  }

  template<class U, class G _ENABLE_IF(
    std::is_constructible_v<T, const U&> &&
    std::is_constructible_v<E, const G&> &&
    !std::is_constructible_v<T, expected<U, G>&> &&
    !std::is_constructible_v<T, expected<U, G>&&> &&
    !std::is_constructible_v<T, const expected<U, G>&> &&
    !std::is_constructible_v<T, const expected<U, G>&&> &&
    !std::is_convertible_v<expected<U, G>&, T> &&
    !std::is_convertible_v<expected<U, G>&&, T> &&
    !std::is_convertible_v<const expected<U, G>&, T> &&
    !std::is_convertible_v<const expected<U, G>&&, T> &&
    (!std::is_convertible_v<const U&, T> ||
     !std::is_convertible_v<const G&, E>) /* explicit */
  )>
  constexpr explicit expected(expected<U, G>&& rhs) {
    if (rhs.has_value()) var_ = std::move(rhs.value());
    else var_ = unexpected(std::move(rhs.error()));
  }

  template <class U = T _ENABLE_IF(
                std::is_constructible_v<T, U&&> &&
                !std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, std::in_place_t> &&
                !std::is_same_v<expected<T, E>, std::remove_cv_t<std::remove_reference_t<U>>> &&
                !std::is_same_v<unexpected<E>, std::remove_cv_t<std::remove_reference_t<U>>> &&
                std::is_convertible_v<U&&, T> /* non-explicit */
                )>
  // NOLINTNEXTLINE(google-explicit-constructor,bugprone-forwarding-reference-overload)
  constexpr expected(U&& v) : var_(std::in_place_index<0>, std::forward<U>(v)) {}

  template <class U = T _ENABLE_IF(
                std::is_constructible_v<T, U&&> &&
                !std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, std::in_place_t> &&
                !std::is_same_v<expected<T, E>, std::remove_cv_t<std::remove_reference_t<U>>> &&
                !std::is_same_v<unexpected<E>, std::remove_cv_t<std::remove_reference_t<U>>> &&
                !std::is_convertible_v<U&&, T> /* explicit */
                )>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  constexpr explicit expected(U&& v) : var_(std::in_place_index<0>, T(std::forward<U>(v))) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, const G&> &&
    std::is_convertible_v<const G&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(const unexpected<G>& e)
  : var_(std::in_place_index<1>, e.value()) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, const G&> &&
    !std::is_convertible_v<const G&, E> /* explicit */
  )>
  constexpr explicit expected(const unexpected<G>& e)
  : var_(std::in_place_index<1>, E(e.value())) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, G&&> &&
    std::is_convertible_v<G&&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(unexpected<G>&& e)
  : var_(std::in_place_index<1>, std::move(e.value())) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, G&&> &&
    !std::is_convertible_v<G&&, E> /* explicit */
  )>
  constexpr explicit expected(unexpected<G>&& e)
  : var_(std::in_place_index<1>, E(std::move(e.value()))) {}

  template<class... Args _ENABLE_IF(
    std::is_constructible_v<T, Args&&...>
  )>
  constexpr explicit expected(std::in_place_t, Args&&... args)
  : var_(std::in_place_index<0>, std::forward<Args>(args)...) {}

  template<class U, class... Args _ENABLE_IF(
    std::is_constructible_v<T, std::initializer_list<U>&, Args...>
  )>
  constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
  : var_(std::in_place_index<0>, il, std::forward<Args>(args)...) {}

  template<class... Args _ENABLE_IF(
    std::is_constructible_v<E, Args...>
  )>
  constexpr explicit expected(unexpect_t, Args&&... args)
  : var_(unexpected_type(std::forward<Args>(args)...)) {}

  template<class U, class... Args _ENABLE_IF(
    std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  )>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
  : var_(unexpected_type(il, std::forward<Args>(args)...)) {}

  // destructor
  ~expected() = default;

  // assignment
  // Note: SFNAIE doesn't work here because assignment operator should be
  // non-template. We could workaround this by defining a templated parent class
  // having the assignment operator. This incomplete implementation however
  // doesn't allow us to copy assign expected<T,E> even when T is non-copy
  // assignable. The copy assignment will fail by the underlying std::variant
  // anyway though the error message won't be clear.
  expected& operator=(const expected& rhs) = default;

  // Note for SFNAIE above applies to here as well
  expected& operator=(expected&& rhs) noexcept(
      std::is_nothrow_move_assignable_v<T>&& std::is_nothrow_move_assignable_v<E>) = default;

  template <class U = T _ENABLE_IF(
                !std::is_void_v<T> &&
                !std::is_same_v<expected<T, E>, std::remove_cv_t<std::remove_reference_t<U>>> &&
                !std::conjunction_v<std::is_scalar<T>, std::is_same<T, std::decay_t<U>>> &&
                std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
                std::is_nothrow_move_constructible_v<E>)>
  expected& operator=(U&& rhs) {
    var_ = T(std::forward<U>(rhs));
    return *this;
  }

  template<class G = E>
  expected& operator=(const unexpected<G>& rhs) {
    var_ = rhs;
    return *this;
  }

  template<class G = E _ENABLE_IF(
    std::is_nothrow_move_constructible_v<G> &&
    std::is_move_assignable_v<G>
  )>
  expected& operator=(unexpected<G>&& rhs) {
    var_ = std::move(rhs);
    return *this;
  }

  // modifiers
  template<class... Args _ENABLE_IF(
    std::is_nothrow_constructible_v<T, Args...>
  )>
  T& emplace(Args&&... args) {
    expected(std::in_place, std::forward<Args>(args)...).swap(*this);
    return value();
  }

  template<class U, class... Args _ENABLE_IF(
    std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
  )>
  T& emplace(std::initializer_list<U> il, Args&&... args) {
    expected(std::in_place, il, std::forward<Args>(args)...).swap(*this);
    return value();
  }

  // swap
  template<typename U = T, typename = std::enable_if_t<(
    std::is_swappable_v<U> &&
    std::is_swappable_v<E> &&
    (std::is_move_constructible_v<U> ||
     std::is_move_constructible_v<E>))>>
  void swap(expected& rhs) noexcept(
    std::is_nothrow_move_constructible_v<T> &&
    std::is_nothrow_swappable_v<T> &&
    std::is_nothrow_move_constructible_v<E> &&
    std::is_nothrow_swappable_v<E>) {
    var_.swap(rhs.var_);
  }

  // observers
  constexpr const T* operator->() const { return std::addressof(value()); }
  constexpr T* operator->() { return std::addressof(value()); }
  constexpr const T& operator*() const& { return value(); }
  constexpr T& operator*() & { return value(); }
  constexpr const T&& operator*() const&& { return std::move(std::get<T>(var_)); }
  constexpr T&& operator*() && { return std::move(std::get<T>(var_)); }

  constexpr bool has_value() const noexcept { return var_.index() == 0; }
  constexpr bool ok() const noexcept { return has_value(); }

  constexpr const T& value() const& { return std::get<T>(var_); }
  constexpr T& value() & { return std::get<T>(var_); }
  constexpr const T&& value() const&& { return std::move(std::get<T>(var_)); }
  constexpr T&& value() && { return std::move(std::get<T>(var_)); }

  constexpr const E& error() const& { return std::get<unexpected_type>(var_).value(); }
  constexpr E& error() & { return std::get<unexpected_type>(var_).value(); }
  constexpr const E&& error() const&& { return std::move(std::get<unexpected_type>(var_)).value(); }
  constexpr E&& error() && { return std::move(std::get<unexpected_type>(var_)).value(); }

  template<class U _ENABLE_IF(
    std::is_copy_constructible_v<T> &&
    std::is_convertible_v<U, T>
  )>
  constexpr T value_or(U&& v) const& {
    if (has_value()) return value();
    else return static_cast<T>(std::forward<U>(v));
  }

  template<class U _ENABLE_IF(
    std::is_move_constructible_v<T> &&
    std::is_convertible_v<U, T>
  )>
  constexpr T value_or(U&& v) && {
    if (has_value()) return std::move(value());
    else return static_cast<T>(std::forward<U>(v));
  }

  // expected equality operators
  template<class T1, class E1, class T2, class E2>
  friend constexpr bool operator==(const expected<T1, E1>& x, const expected<T2, E2>& y);
  template<class T1, class E1, class T2, class E2>
  friend constexpr bool operator!=(const expected<T1, E1>& x, const expected<T2, E2>& y);

  // Comparison with unexpected<E>
  template<class T1, class E1, class E2>
  friend constexpr bool operator==(const expected<T1, E1>&, const unexpected<E2>&);
  template<class T1, class E1, class E2>
  friend constexpr bool operator==(const unexpected<E2>&, const expected<T1, E1>&);
  template<class T1, class E1, class E2>
  friend constexpr bool operator!=(const expected<T1, E1>&, const unexpected<E2>&);
  template<class T1, class E1, class E2>
  friend constexpr bool operator!=(const unexpected<E2>&, const expected<T1, E1>&);

  // Specialized algorithms
  template<class T1, class E1>
  friend void swap(expected<T1, E1>& x, expected<T1, E1>& y) noexcept(noexcept(x.swap(y)));

 private:
  std::variant<value_type, unexpected_type> var_;
};

template<class T1, class E1, class T2, class E2>
constexpr bool operator==(const expected<T1, E1>& x, const expected<T2, E2>& y) {
  if (x.has_value() != y.has_value()) return false;
  if (!x.has_value()) return x.error() == y.error();
  return *x == *y;
}

template<class T1, class E1, class T2, class E2>
constexpr bool operator!=(const expected<T1, E1>& x, const expected<T2, E2>& y) {
  return !(x == y);
}

// Comparison with unexpected<E>
template<class T1, class E1, class E2>
constexpr bool operator==(const expected<T1, E1>& x, const unexpected<E2>& y) {
  return !x.has_value() && (x.error() == y.value());
}
template<class T1, class E1, class E2>
constexpr bool operator==(const unexpected<E2>& x, const expected<T1, E1>& y) {
  return !y.has_value() && (x.value() == y.error());
}
template<class T1, class E1, class E2>
constexpr bool operator!=(const expected<T1, E1>& x, const unexpected<E2>& y) {
  return x.has_value() || (x.error() != y.value());
}
template<class T1, class E1, class E2>
constexpr bool operator!=(const unexpected<E2>& x, const expected<T1, E1>& y) {
  return y.has_value() || (x.value() != y.error());
}

template<class T1, class E1>
void swap(expected<T1, E1>& x, expected<T1, E1>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}

template<class E>
class _NODISCARD_ expected<void, E> {
 public:
  using value_type = void;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  // constructors
  constexpr expected() = default;
  constexpr expected(const expected& rhs) = default;
  constexpr expected(expected&& rhs) noexcept = default;

  template<class U, class G _ENABLE_IF(
    std::is_void_v<U> &&
    std::is_convertible_v<const G&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(const expected<U, G>& rhs) {
    if (!rhs.has_value()) var_ = unexpected(rhs.error());
  }

  template<class U, class G _ENABLE_IF(
    std::is_void_v<U> &&
    !std::is_convertible_v<const G&, E> /* explicit */
  )>
  constexpr explicit expected(const expected<U, G>& rhs) {
    if (!rhs.has_value()) var_ = unexpected(rhs.error());
  }

  template<class U, class G _ENABLE_IF(
    std::is_void_v<U> &&
    std::is_convertible_v<const G&&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(expected<U, G>&& rhs) {
    if (!rhs.has_value()) var_ = unexpected(std::move(rhs.error()));
  }

  template<class U, class G _ENABLE_IF(
    std::is_void_v<U> &&
    !std::is_convertible_v<const G&&, E> /* explicit */
  )>
  constexpr explicit expected(expected<U, G>&& rhs) {
    if (!rhs.has_value()) var_ = unexpected(std::move(rhs.error()));
  }

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, const G&> &&
    std::is_convertible_v<const G&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(const unexpected<G>& e)
  : var_(std::in_place_index<1>, e.value()) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, const G&> &&
    !std::is_convertible_v<const G&, E> /* explicit */
  )>
  constexpr explicit expected(const unexpected<G>& e)
  : var_(std::in_place_index<1>, E(e.value())) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, G&&> &&
    std::is_convertible_v<G&&, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr expected(unexpected<G>&& e)
  : var_(std::in_place_index<1>, std::move(e.value())) {}

  template<class G = E _ENABLE_IF(
    std::is_constructible_v<E, G&&> &&
    !std::is_convertible_v<G&&, E> /* explicit */
  )>
  constexpr explicit expected(unexpected<G>&& e)
  : var_(std::in_place_index<1>, E(std::move(e.value()))) {}

  template<class... Args _ENABLE_IF(
    sizeof...(Args) == 0
  )>
  constexpr explicit expected(std::in_place_t, Args&&...) {}

  template<class... Args _ENABLE_IF(
    std::is_constructible_v<E, Args...>
  )>
  constexpr explicit expected(unexpect_t, Args&&... args)
  : var_(unexpected_type(std::forward<Args>(args)...)) {}

  template<class U, class... Args _ENABLE_IF(
    std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  )>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
  : var_(unexpected_type(il, std::forward<Args>(args)...)) {}

  // destructor
  ~expected() = default;

  // assignment
  // Note: SFNAIE doesn't work here because assignment operator should be
  // non-template. We could workaround this by defining a templated parent class
  // having the assignment operator. This incomplete implementation however
  // doesn't allow us to copy assign expected<T,E> even when T is non-copy
  // assignable. The copy assignment will fail by the underlying std::variant
  // anyway though the error message won't be clear.
  expected& operator=(const expected& rhs) = default;

  // Note for SFNAIE above applies to here as well
  expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_assignable_v<E>) = default;

  template<class G = E>
  expected& operator=(const unexpected<G>& rhs) {
    var_ = rhs;
    return *this;
  }

  template<class G = E _ENABLE_IF(
    std::is_nothrow_move_constructible_v<G> &&
    std::is_move_assignable_v<G>
  )>
  expected& operator=(unexpected<G>&& rhs) {
    var_ = std::move(rhs);
    return *this;
  }

  // modifiers
  void emplace() {
    var_ = std::monostate();
  }

  // swap
  template<typename = std::enable_if_t<
    std::is_swappable_v<E>>
  >
  void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E>) {
    var_.swap(rhs.var_);
  }

  // observers
  constexpr bool has_value() const noexcept { return var_.index() == 0; }
  constexpr bool ok() const noexcept { return has_value(); }

  constexpr void value() const& { if (!has_value()) std::get<0>(var_); }

  constexpr const E& error() const& { return std::get<unexpected_type>(var_).value(); }
  constexpr E& error() & { return std::get<unexpected_type>(var_).value(); }
  constexpr const E&& error() const&& { return std::move(std::get<unexpected_type>(var_)).value(); }
  constexpr E&& error() && { return std::move(std::get<unexpected_type>(var_)).value(); }

  // expected equality operators
  template<class E1, class E2>
  friend constexpr bool operator==(const expected<void, E1>& x, const expected<void, E2>& y);

  // Specialized algorithms
  template<class T1, class E1>
  friend void swap(expected<T1, E1>& x, expected<T1, E1>& y) noexcept(noexcept(x.swap(y)));

 private:
  std::variant<std::monostate, unexpected_type> var_;
};

template<class E1, class E2>
constexpr bool operator==(const expected<void, E1>& x, const expected<void, E2>& y) {
  if (x.has_value() != y.has_value()) return false;
  if (!x.has_value()) return x.error() == y.error();
  return true;
}

template<class T1, class E1, class E2>
constexpr bool operator==(const expected<T1, E1>& x, const expected<void, E2>& y) {
  if (x.has_value() != y.has_value()) return false;
  if (!x.has_value()) return x.error() == y.error();
  return false;
}

template<class E1, class T2, class E2>
constexpr bool operator==(const expected<void, E1>& x, const expected<T2, E2>& y) {
  if (x.has_value() != y.has_value()) return false;
  if (!x.has_value()) return x.error() == y.error();
  return false;
}

template<class E>
class unexpected {
 public:
  // constructors
  constexpr unexpected(const unexpected&) = default;
  constexpr unexpected(unexpected&&) noexcept(std::is_nothrow_move_constructible_v<E>) = default;

  template <class Err = E _ENABLE_IF(
                std::is_constructible_v<E, Err> &&
                !std::is_same_v<std::remove_cv_t<std::remove_reference_t<E>>, std::in_place_t> &&
                !std::is_same_v<std::remove_cv_t<std::remove_reference_t<E>>, unexpected>)>
  // NOLINTNEXTLINE(google-explicit-constructor,bugprone-forwarding-reference-overload)
  constexpr unexpected(Err&& e) : val_(std::forward<Err>(e)) {}

  template<class U, class... Args _ENABLE_IF(
    std::is_constructible_v<E, std::initializer_list<U>&, Args...>
  )>
  constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
  : val_(il, std::forward<Args>(args)...) {}

  template<class Err _ENABLE_IF(
    std::is_constructible_v<E, Err> &&
    !std::is_constructible_v<E, unexpected<Err>&> &&
    !std::is_constructible_v<E, unexpected<Err>> &&
    !std::is_constructible_v<E, const unexpected<Err>&> &&
    !std::is_constructible_v<E, const unexpected<Err>> &&
    !std::is_convertible_v<unexpected<Err>&, E> &&
    !std::is_convertible_v<unexpected<Err>, E> &&
    !std::is_convertible_v<const unexpected<Err>&, E> &&
    !std::is_convertible_v<const unexpected<Err>, E> &&
    std::is_convertible_v<Err, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr unexpected(const unexpected<Err>& rhs)
  : val_(rhs.value()) {}

  template<class Err _ENABLE_IF(
    std::is_constructible_v<E, Err> &&
    !std::is_constructible_v<E, unexpected<Err>&> &&
    !std::is_constructible_v<E, unexpected<Err>> &&
    !std::is_constructible_v<E, const unexpected<Err>&> &&
    !std::is_constructible_v<E, const unexpected<Err>> &&
    !std::is_convertible_v<unexpected<Err>&, E> &&
    !std::is_convertible_v<unexpected<Err>, E> &&
    !std::is_convertible_v<const unexpected<Err>&, E> &&
    !std::is_convertible_v<const unexpected<Err>, E> &&
    !std::is_convertible_v<Err, E> /* explicit */
  )>
  constexpr explicit unexpected(const unexpected<Err>& rhs)
  : val_(E(rhs.value())) {}

  template<class Err _ENABLE_IF(
    std::is_constructible_v<E, Err> &&
    !std::is_constructible_v<E, unexpected<Err>&> &&
    !std::is_constructible_v<E, unexpected<Err>> &&
    !std::is_constructible_v<E, const unexpected<Err>&> &&
    !std::is_constructible_v<E, const unexpected<Err>> &&
    !std::is_convertible_v<unexpected<Err>&, E> &&
    !std::is_convertible_v<unexpected<Err>, E> &&
    !std::is_convertible_v<const unexpected<Err>&, E> &&
    !std::is_convertible_v<const unexpected<Err>, E> &&
    std::is_convertible_v<Err, E> /* non-explicit */
  )>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr unexpected(unexpected<Err>&& rhs)
  : val_(std::move(rhs.value())) {}

  template<class Err _ENABLE_IF(
    std::is_constructible_v<E, Err> &&
    !std::is_constructible_v<E, unexpected<Err>&> &&
    !std::is_constructible_v<E, unexpected<Err>> &&
    !std::is_constructible_v<E, const unexpected<Err>&> &&
    !std::is_constructible_v<E, const unexpected<Err>> &&
    !std::is_convertible_v<unexpected<Err>&, E> &&
    !std::is_convertible_v<unexpected<Err>, E> &&
    !std::is_convertible_v<const unexpected<Err>&, E> &&
    !std::is_convertible_v<const unexpected<Err>, E> &&
    !std::is_convertible_v<Err, E> /* explicit */
  )>
  constexpr explicit unexpected(unexpected<Err>&& rhs)
  : val_(E(std::move(rhs.value()))) {}

  // assignment
  constexpr unexpected& operator=(const unexpected&) = default;
  constexpr unexpected& operator=(unexpected&&) noexcept(std::is_nothrow_move_assignable_v<E>) =
      default;
  template<class Err = E>
  constexpr unexpected& operator=(const unexpected<Err>& rhs) {
    val_ = rhs.value();
    return *this;
  }
  template<class Err = E>
  constexpr unexpected& operator=(unexpected<Err>&& rhs) {
    val_ = std::forward<E>(rhs.value());
    return *this;
  }

  // observer
  constexpr const E& value() const& noexcept { return val_; }
  constexpr E& value() & noexcept { return val_; }
  constexpr const E&& value() const&& noexcept { return std::move(val_); }
  constexpr E&& value() && noexcept { return std::move(val_); }

  void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
    std::swap(val_, other.val_);
  }

  template<class E1, class E2>
  friend constexpr bool
  operator==(const unexpected<E1>& e1, const unexpected<E2>& e2);
  template<class E1, class E2>
  friend constexpr bool
  operator!=(const unexpected<E1>& e1, const unexpected<E2>& e2);

  template<class E1>
  friend void swap(unexpected<E1>& x, unexpected<E1>& y) noexcept(noexcept(x.swap(y)));

 private:
  E val_;
};

template<class E1, class E2>
constexpr bool
operator==(const unexpected<E1>& e1, const unexpected<E2>& e2) {
  return e1.value() == e2.value();
}

template<class E1, class E2>
constexpr bool
operator!=(const unexpected<E1>& e1, const unexpected<E2>& e2) {
  return e1.value() != e2.value();
}

template<class E1>
void swap(unexpected<E1>& x, unexpected<E1>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}

// TODO: bad_expected_access class

#undef _ENABLE_IF
#undef _NODISCARD_

}  // namespace base
}  // namespace android
