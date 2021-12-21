//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// template <class Tp> struct __is_pathable

// [path.req]
// In addition to the requirements (5), function template parameters named
// `Source` shall be one of:
// * basic_string<_ECharT, _Traits, _Alloc>
// * InputIterator with a value_type of _ECharT
// * A character array, which points to a NTCTS after array-to-pointer decay.


#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "min_allocator.h"
#include "constexpr_char_traits.hpp"

using fs::__is_pathable;

template <class Tp>
struct Identity { typedef Tp type; };

template <class Source>
Identity<Source> CheckSourceType(Source const&);

template <class Tp>
using GetSourceType = typename decltype(CheckSourceType(std::declval<Tp>()))::type;

template <class Tp, class Exp,
          class ExpQual = typename std::remove_const<Exp>::type>
using CheckPass = std::is_same<ExpQual, GetSourceType<Tp>>;

template <class Source>
using CheckPassSource = std::integral_constant<bool,
        CheckPass<Source&,        Source>::value &&
        CheckPass<Source const&,  Source>::value &&
        CheckPass<Source&&,       Source>::value &&
        CheckPass<Source const&&, Source>::value
  >;

template <class CharT>
struct MakeTestType {
  using value_type = CharT;
  using string_type = std::basic_string<CharT>;
  using string_type2 = std::basic_string<CharT, std::char_traits<CharT>, min_allocator<CharT>>;
  using string_view_type = std::basic_string_view<CharT>;
  using string_view_type2 = std::basic_string_view<CharT, constexpr_char_traits<CharT>>;
  using cstr_type = CharT* const;
  using const_cstr_type = const CharT*;
  using array_type = CharT[25];
  using const_array_type = const CharT[25];
  using iter_type = input_iterator<CharT*>;
  using bad_iter_type = input_iterator<signed char*>;

  template <class TestT>
  static void AssertPathable() {
    static_assert(__is_pathable<TestT>::value, "");
    static_assert(CheckPassSource<TestT>::value, "cannot pass as Source const&");
    ASSERT_SAME_TYPE(CharT, typename __is_pathable<TestT>::__char_type);
  }

  template <class TestT>
  static void AssertNotPathable() {
    static_assert(!__is_pathable<TestT>::value, "");
  }

  static void Test() {
    AssertPathable<string_type>();
    AssertPathable<string_type2>();
    AssertPathable<string_view_type>();
    AssertPathable<string_view_type2>();
    AssertPathable<cstr_type>();
    AssertPathable<const_cstr_type>();
    AssertPathable<array_type>();
    AssertPathable<const_array_type>();
    AssertPathable<iter_type>();

    AssertNotPathable<CharT>();
    AssertNotPathable<bad_iter_type>();
    AssertNotPathable<signed char*>();
  }
};

int main() {
  MakeTestType<char>::Test();
  MakeTestType<wchar_t>::Test();
  MakeTestType<char16_t>::Test();
  MakeTestType<char32_t>::Test();
}
