//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-5, apple-clang-9
// UNSUPPORTED: libcpp-no-deduction-guides
// Clang 5 will generate bad implicit deduction guides
//	Specifically, for the copy constructor.


// template <class T, class... U>
//   array(T, U...) -> array<T, 1 + sizeof...(U)>;
//
//  Requires: (is_same_v<T, U> && ...) is true. Otherwise the program is ill-formed.


#include <array>
#include <cassert>
#include <cstddef>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

#include "test_macros.h"

int main()
{
//  Test the explicit deduction guides
    {
    std::array arr{1,2,3};  // array(T, U...)
    static_assert(std::is_same_v<decltype(arr), std::array<int, 3>>, "");
    assert(arr[0] == 1);
    assert(arr[1] == 2);
    assert(arr[2] == 3);
    }

    {
    const long l1 = 42;
    std::array arr{1L, 4L, 9L, l1}; // array(T, U...)
    static_assert(std::is_same_v<decltype(arr)::value_type, long>, "");
    static_assert(arr.size() == 4, "");
    assert(arr[0] == 1);
    assert(arr[1] == 4);
    assert(arr[2] == 9);
    assert(arr[3] == l1);
    }

//  Test the implicit deduction guides
  {
  std::array<double, 2> source = {4.0, 5.0};
  std::array arr(source);   // array(array)
    static_assert(std::is_same_v<decltype(arr), decltype(source)>, "");
    static_assert(std::is_same_v<decltype(arr), std::array<double, 2>>, "");
    assert(arr[0] == 4.0);
    assert(arr[1] == 5.0);
  }
}
