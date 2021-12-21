//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// placeholders
// The placeholders are constexpr in C++17 and beyond.
// libc++ provides constexpr placeholders in C++11 and beyond.

#include <functional>
#include <type_traits>

#include "test_macros.h"

template <class T>
void
test(const T& t)
{
    // Test default constructible.
    T t2;
    ((void)t2);
    // Test copy constructible.
    T t3 = t;
    ((void)t3);
    static_assert(std::is_nothrow_copy_constructible<T>::value, "");
    static_assert(std::is_nothrow_move_constructible<T>::value, "");
}

#if TEST_STD_VER >= 11
constexpr decltype(std::placeholders::_1)  default1{};
constexpr decltype(std::placeholders::_2)  default2{};
constexpr decltype(std::placeholders::_3)  default3{};
constexpr decltype(std::placeholders::_4)  default4{};
constexpr decltype(std::placeholders::_5)  default5{};
constexpr decltype(std::placeholders::_6)  default6{};
constexpr decltype(std::placeholders::_7)  default7{};
constexpr decltype(std::placeholders::_8)  default8{};
constexpr decltype(std::placeholders::_9)  default9{};
constexpr decltype(std::placeholders::_10) default10{};

constexpr decltype(std::placeholders::_1)  cp1 = std::placeholders::_1;
constexpr decltype(std::placeholders::_2)  cp2 = std::placeholders::_2;
constexpr decltype(std::placeholders::_3)  cp3 = std::placeholders::_3;
constexpr decltype(std::placeholders::_4)  cp4 = std::placeholders::_4;
constexpr decltype(std::placeholders::_5)  cp5 = std::placeholders::_5;
constexpr decltype(std::placeholders::_6)  cp6 = std::placeholders::_6;
constexpr decltype(std::placeholders::_7)  cp7 = std::placeholders::_7;
constexpr decltype(std::placeholders::_8)  cp8 = std::placeholders::_8;
constexpr decltype(std::placeholders::_9)  cp9 = std::placeholders::_9;
constexpr decltype(std::placeholders::_10) cp10 = std::placeholders::_10;
#endif // TEST_STD_VER >= 11

void use_placeholders_to_prevent_unused_warning() {
#if TEST_STD_VER >= 11
  ((void)cp1);
  ((void)cp2);
  ((void)cp3);
  ((void)cp4);
  ((void)cp5);
  ((void)cp6);
  ((void)cp7);
  ((void)cp8);
  ((void)cp9);
  ((void)cp10);
  ((void)default1);
  ((void)default2);
  ((void)default3);
  ((void)default4);
  ((void)default5);
  ((void)default6);
  ((void)default7);
  ((void)default8);
  ((void)default9);
  ((void)default10);
#endif
}

int main()
{
    use_placeholders_to_prevent_unused_warning();
    test(std::placeholders::_1);
    test(std::placeholders::_2);
    test(std::placeholders::_3);
    test(std::placeholders::_4);
    test(std::placeholders::_5);
    test(std::placeholders::_6);
    test(std::placeholders::_7);
    test(std::placeholders::_8);
    test(std::placeholders::_9);
    test(std::placeholders::_10);
}
