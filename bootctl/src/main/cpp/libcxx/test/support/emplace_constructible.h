#ifndef TEST_SUPPORT_EMPLACE_CONSTRUCTIBLE_H
#define TEST_SUPPORT_EMPLACE_CONSTRUCTIBLE_H

#include "test_macros.h"

#if TEST_STD_VER >= 11
template <class T>
struct EmplaceConstructible {
  T value;
  explicit EmplaceConstructible(T xvalue) : value(xvalue) {}
  EmplaceConstructible(EmplaceConstructible const&) = delete;
};

template <class T>
struct EmplaceConstructibleAndMoveInsertable {
  int copied = 0;
  T value;
  explicit EmplaceConstructibleAndMoveInsertable(T xvalue) : value(xvalue) {}

  EmplaceConstructibleAndMoveInsertable(
      EmplaceConstructibleAndMoveInsertable&& Other)
      : copied(Other.copied + 1), value(std::move(Other.value)) {}
};

template <class T>
struct EmplaceConstructibleAndMoveable {
  int copied = 0;
  int assigned = 0;
  T value;
  explicit EmplaceConstructibleAndMoveable(T xvalue) noexcept : value(xvalue) {}

  EmplaceConstructibleAndMoveable(EmplaceConstructibleAndMoveable&& Other)
      noexcept : copied(Other.copied + 1),
                 value(std::move(Other.value)) {}

  EmplaceConstructibleAndMoveable&
  operator=(EmplaceConstructibleAndMoveable&& Other) noexcept {
    copied = Other.copied;
    assigned = Other.assigned + 1;
    value = std::move(Other.value);
    return *this;
  }
};

template <class T>
struct EmplaceConstructibleMoveableAndAssignable {
  int copied = 0;
  int assigned = 0;
  T value;
  explicit EmplaceConstructibleMoveableAndAssignable(T xvalue) noexcept
      : value(xvalue) {}

  EmplaceConstructibleMoveableAndAssignable(
      EmplaceConstructibleMoveableAndAssignable&& Other) noexcept
      : copied(Other.copied + 1),
        value(std::move(Other.value)) {}

  EmplaceConstructibleMoveableAndAssignable&
  operator=(EmplaceConstructibleMoveableAndAssignable&& Other) noexcept {
    copied = Other.copied;
    assigned = Other.assigned + 1;
    value = std::move(Other.value);
    return *this;
  }

  EmplaceConstructibleMoveableAndAssignable& operator=(T xvalue) {
    value = std::move(xvalue);
    ++assigned;
    return *this;
  }
};
#endif

#endif // TEST_SUPPORT_EMPLACE_CONSTRUCTIBLE_H
