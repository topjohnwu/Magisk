//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// XFAIL: libcpp-no-deduction-guides

// template<class InputIterator>
//   basic_string(InputIterator begin, InputIterator end,
//   const Allocator& a = Allocator());

// template<class charT,
//          class traits,
//          class Allocator = allocator<charT>
//          >
// basic_string(basic_string_view<charT, traits>, const Allocator& = Allocator())
//   -> basic_string<charT, traits, Allocator>;
//
//  The deduction guide shall not participate in overload resolution if Allocator
//  is a type that does not qualify as an allocator.


#include <string>
#include <string_view>
#include <iterator>
#include <memory>
#include <type_traits>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "../input_iterator.h"
#include "min_allocator.h"

int main()
{
    {
    std::string_view sv = "12345678901234";
    std::basic_string s1(sv);
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char>>, "");
    static_assert(std::is_same_v<S::allocator_type,   std::allocator<char>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }

    {
    std::string_view sv = "12345678901234";
    std::basic_string s1{sv, std::allocator<char>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char>>, "");
    static_assert(std::is_same_v<S::allocator_type,   std::allocator<char>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }
    {
    std::wstring_view sv = L"12345678901234";
    std::basic_string s1{sv, test_allocator<wchar_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      wchar_t>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<wchar_t>>, "");
    static_assert(std::is_same_v<S::allocator_type,   test_allocator<wchar_t>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    {
    std::u8string_view sv = u8"12345678901234";
    std::basic_string s1{sv, min_allocator<char8_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char8_t>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char8_t>>, "");
    static_assert(std::is_same_v<S::allocator_type,    min_allocator<char8_t>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }
#endif
    {
    std::u16string_view sv = u"12345678901234";
    std::basic_string s1{sv, min_allocator<char16_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char16_t>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char16_t>>, "");
    static_assert(std::is_same_v<S::allocator_type,    min_allocator<char16_t>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }
    {
    std::u32string_view sv = U"12345678901234";
    std::basic_string s1{sv, explicit_allocator<char32_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                        char32_t>,  "");
    static_assert(std::is_same_v<S::traits_type,      std::char_traits<char32_t>>, "");
    static_assert(std::is_same_v<S::allocator_type, explicit_allocator<char32_t>>, "");
    assert(s1.size() == sv.size());
    assert(s1.compare(0, s1.size(), sv.data(), s1.size()) == 0);
    }
}
