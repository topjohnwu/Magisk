//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class... Types> class tuple;

// ~tuple();

// C++17 added:
//   The destructor of tuple shall be a trivial destructor
//     if (is_trivially_destructible_v<Types> && ...) is true.

#include <tuple>
#include <string>
#include <cassert>
#include <type_traits>

int main()
{
  static_assert(std::is_trivially_destructible<
      std::tuple<> >::value, "");
  static_assert(std::is_trivially_destructible<
      std::tuple<void*> >::value, "");
  static_assert(std::is_trivially_destructible<
      std::tuple<int, float> >::value, "");
  static_assert(!std::is_trivially_destructible<
      std::tuple<std::string> >::value, "");
  static_assert(!std::is_trivially_destructible<
      std::tuple<int, std::string> >::value, "");
}
