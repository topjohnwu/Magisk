// -*- C++ -*-
//===------------------------------ span ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <span>

//   template<class T, size_t N>
//     span(T (&)[N]) -> span<T, N>;
//
//   template<class T, size_t N>
//     span(array<T, N>&) -> span<T, N>;
//
//   template<class T, size_t N>
//     span(const array<T, N>&) -> span<const T, N>;
//
//   template<class Container>
//     span(Container&) -> span<typename Container::value_type>;
//
//   template<class Container>
//     span(const Container&) -> span<const typename Container::value_type>;



#include <span>
#include <algorithm>
#include <array>
#include <cassert>
#include <string>
#include <type_traits>

#include "test_macros.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main ()
{
    {
    int arr[] = {1,2,3};
    std::span s{arr};
    using S = decltype(s);
    ASSERT_SAME_TYPE(S, std::span<int, 3>);
    assert((std::equal(std::begin(arr), std::end(arr), s.begin(), s.end())));
    }

    {
    std::array<double, 4> arr = {1.0, 2.0, 3.0, 4.0};
    std::span s{arr};
    using S = decltype(s);
    ASSERT_SAME_TYPE(S, std::span<double, 4>);
    assert((std::equal(std::begin(arr), std::end(arr), s.begin(), s.end())));
    }

    {
    const std::array<long, 5> arr = {4, 5, 6, 7, 8};
    std::span s{arr};
    using S = decltype(s);
    ASSERT_SAME_TYPE(S, std::span<const long, 5>);
    assert((std::equal(std::begin(arr), std::end(arr), s.begin(), s.end())));
    }

    {
    std::string str{"ABCDE"};
    std::span s{str};
    using S = decltype(s);
    ASSERT_SAME_TYPE(S, std::span<char>);
    assert((size_t)s.size() == str.size());
    assert((std::equal(s.begin(), s.end(), std::begin(s), std::end(s))));
    }

    {
    const std::string str{"QWERTYUIOP"};
    std::span s{str};
    using S = decltype(s);
    ASSERT_SAME_TYPE(S, std::span<const char>);
    assert((size_t)s.size() == str.size());
    assert((std::equal(s.begin(), s.end(), std::begin(s), std::end(s))));
    }
}
