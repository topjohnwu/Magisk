//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// template<size_t I, class T1, class T2>
//     const typename tuple_element<I, std::pair<T1, T2> >::type&&
//     get(const pair<T1, T2>&&);

// UNSUPPORTED: c++98, c++03

#include <utility>
#include <memory>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::pair<std::unique_ptr<int>, short> P;
    const P p(std::unique_ptr<int>(new int(3)), static_cast<short>(4));
    static_assert(std::is_same<const std::unique_ptr<int>&&, decltype(std::get<0>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(p))), "");
    const std::unique_ptr<int>&& ptr = std::get<0>(std::move(p));
    assert(*ptr == 3);
    }

    {
    int x = 42;
    int const y = 43;
    std::pair<int&, int const&> const p(x, y);
    static_assert(std::is_same<int&, decltype(std::get<0>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(p))), "");
    static_assert(std::is_same<int const&, decltype(std::get<1>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<1>(std::move(p))), "");
    }

    {
    int x = 42;
    int const y = 43;
    std::pair<int&&, int const&&> const p(std::move(x), std::move(y));
    static_assert(std::is_same<int&&, decltype(std::get<0>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<0>(std::move(p))), "");
    static_assert(std::is_same<int const&&, decltype(std::get<1>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<1>(std::move(p))), "");
    }

#if TEST_STD_VER > 11
    {
    typedef std::pair<int, short> P;
    constexpr const P p1(3, static_cast<short>(4));
    static_assert(std::get<0>(std::move(p1)) == 3, "");
    static_assert(std::get<1>(std::move(p1)) == 4, "");
    }
#endif
}
