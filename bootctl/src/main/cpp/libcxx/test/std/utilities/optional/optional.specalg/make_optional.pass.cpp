//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// template <class T>
//   constexpr optional<decay_t<T>> make_optional(T&& v);

#include <optional>
#include <string>
#include <memory>
#include <cassert>

#include "test_macros.h"

int main()
{
    using std::optional;
    using std::make_optional;
    {
        int arr[10]; ((void)arr);
        ASSERT_SAME_TYPE(decltype(make_optional(arr)), optional<int*>);
    }
    {
        constexpr auto opt = make_optional(2);
        ASSERT_SAME_TYPE(decltype(opt), const optional<int>);
        static_assert(opt.value() == 2);
    }
    {
        optional<int> opt = make_optional(2);
        assert(*opt == 2);
    }
    {
        std::string s("123");
        optional<std::string> opt = make_optional(s);
        assert(*opt == s);
    }
    {
        std::unique_ptr<int> s(new int(3));
        optional<std::unique_ptr<int>> opt = make_optional(std::move(s));
        assert(**opt == 3);
        assert(s == nullptr);
    }
}
