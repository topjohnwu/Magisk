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

// template<class InputIterator,
//      class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//  basic_string(InputIterator, InputIterator, Allocator = Allocator())
//    -> basic_string<typename iterator_traits<InputIterator>::value_type,
//                 char_traits<typename iterator_traits<InputIterator>::value_type>,
//                 Allocator>;
//
//  The deduction guide shall not participate in overload resolution if InputIterator
//  is a type that does not qualify as an input iterator, or if Allocator is a type
//  that does not qualify as an allocator.


#include <string>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "../input_iterator.h"
#include "min_allocator.h"

int main()
{
    {
    const char* s = "12345678901234";
    std::basic_string s1(s, s+10);  // Can't use {} here
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char>>, "");
    static_assert(std::is_same_v<S::allocator_type,   std::allocator<char>>, "");
    assert(s1.size() == 10);
    assert(s1.compare(0, s1.size(), s, s1.size()) == 0);
    }

    {
    const char* s = "12345678901234";
    std::basic_string s1{s, s+10, std::allocator<char>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char>>, "");
    static_assert(std::is_same_v<S::allocator_type,   std::allocator<char>>, "");
    assert(s1.size() == 10);
    assert(s1.compare(0, s1.size(), s, s1.size()) == 0);
    }
    {
    const wchar_t* s = L"12345678901234";
    std::basic_string s1{s, s+10, test_allocator<wchar_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      wchar_t>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<wchar_t>>, "");
    static_assert(std::is_same_v<S::allocator_type,   test_allocator<wchar_t>>, "");
    assert(s1.size() == 10);
    assert(s1.compare(0, s1.size(), s, s1.size()) == 0);
    }
    {
    const char16_t* s = u"12345678901234";
    std::basic_string s1{s, s+10, min_allocator<char16_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                      char16_t>,  "");
    static_assert(std::is_same_v<S::traits_type,    std::char_traits<char16_t>>, "");
    static_assert(std::is_same_v<S::allocator_type,    min_allocator<char16_t>>, "");
    assert(s1.size() == 10);
    assert(s1.compare(0, s1.size(), s, s1.size()) == 0);
    }
    {
    const char32_t* s = U"12345678901234";
    std::basic_string s1{s, s+10, explicit_allocator<char32_t>{}};
    using S = decltype(s1); // what type did we get?
    static_assert(std::is_same_v<S::value_type,                        char32_t>,  "");
    static_assert(std::is_same_v<S::traits_type,      std::char_traits<char32_t>>, "");
    static_assert(std::is_same_v<S::allocator_type, explicit_allocator<char32_t>>, "");
    assert(s1.size() == 10);
    assert(s1.compare(0, s1.size(), s, s1.size()) == 0);
    }
}
