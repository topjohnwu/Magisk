//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <size_t I, class... Types>
//   const typename tuple_element<I, tuple<Types...> >::type&&
//   get(const tuple<Types...>&& t);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <string>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::tuple<int> T;
    const T t(3);
    static_assert(std::is_same<const int&&, decltype(std::get<0>(std::move(t)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(t))), "");
    const int&& i = std::get<0>(std::move(t));
    assert(i == 3);
    }

    {
    typedef std::tuple<std::string, int> T;
    const T t("high", 5);
    static_assert(std::is_same<const std::string&&, decltype(std::get<0>(std::move(t)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(t))), "");
    static_assert(std::is_same<const int&&, decltype(std::get<1>(std::move(t)))>::value, "");
    static_assert(noexcept(std::get<1>(std::move(t))), "");
    const std::string&& s = std::get<0>(std::move(t));
    const int&& i = std::get<1>(std::move(t));
    assert(s == "high");
    assert(i == 5);
    }

    {
    int x = 42;
    int const y = 43;
    std::tuple<int&, int const&> const p(x, y);
    static_assert(std::is_same<int&, decltype(std::get<0>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(p))), "");
    static_assert(std::is_same<int const&, decltype(std::get<1>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<1>(std::move(p))), "");
    }

    {
    int x = 42;
    int const y = 43;
    std::tuple<int&&, int const&&> const p(std::move(x), std::move(y));
    static_assert(std::is_same<int&&, decltype(std::get<0>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(p))), "");
    static_assert(std::is_same<int const&&, decltype(std::get<1>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<1>(std::move(p))), "");
    }

#if TEST_STD_VER > 11
    {
    typedef std::tuple<double, int> T;
    constexpr const T t(2.718, 5);
    static_assert(std::get<0>(std::move(t)) == 2.718, "");
    static_assert(std::get<1>(std::move(t)) == 5, "");
    }
#endif
}
