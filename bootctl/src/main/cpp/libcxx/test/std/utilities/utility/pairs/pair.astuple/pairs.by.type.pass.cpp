//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

#include <utility>
#include <string>
#include <type_traits>
#include <complex>
#include <memory>

#include <cassert>

int main()
{
    typedef std::complex<float> cf;
    {
    auto t1 = std::make_pair<int, cf> ( 42, { 1,2 } );
    assert ( std::get<int>(t1) == 42 );
    assert ( std::get<cf>(t1).real() == 1 );
    assert ( std::get<cf>(t1).imag() == 2 );
    }

    {
    const std::pair<int, const int> p1 { 1, 2 };
    const int &i1 = std::get<int>(p1);
    const int &i2 = std::get<const int>(p1);
    assert ( i1 == 1 );
    assert ( i2 == 2 );
    }

    {
    typedef std::unique_ptr<int> upint;
    std::pair<upint, int> t(upint(new int(4)), 42);
    upint p = std::get<upint>(std::move(t)); // get rvalue
    assert(*p == 4);
    assert(std::get<upint>(t) == nullptr); // has been moved from
    }

    {
    typedef std::unique_ptr<int> upint;
    const std::pair<upint, int> t(upint(new int(4)), 42);
    static_assert(std::is_same<const upint&&, decltype(std::get<upint>(std::move(t)))>::value, "");
    static_assert(noexcept(std::get<upint>(std::move(t))), "");
    static_assert(std::is_same<const int&&, decltype(std::get<int>(std::move(t)))>::value, "");
    static_assert(noexcept(std::get<int>(std::move(t))), "");
    auto&& p = std::get<upint>(std::move(t)); // get const rvalue
    auto&& i = std::get<int>(std::move(t)); // get const rvalue
    assert(*p == 4);
    assert(i == 42);
    assert(std::get<upint>(t) != nullptr);
    }

    {
    int x = 42;
    int const y = 43;
    std::pair<int&, int const&> const p(x, y);
    static_assert(std::is_same<int&, decltype(std::get<int&>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<int&>(std::move(p))), "");
    static_assert(std::is_same<int const&, decltype(std::get<int const&>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<int const&>(std::move(p))), "");
    }

    {
    int x = 42;
    int const y = 43;
    std::pair<int&&, int const&&> const p(std::move(x), std::move(y));
    static_assert(std::is_same<int&&, decltype(std::get<int&&>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<int&&>(std::move(p))), "");
    static_assert(std::is_same<int const&&, decltype(std::get<int const&&>(std::move(p)))>::value, "");
    static_assert(noexcept(std::get<int const&&>(std::move(p))), "");
    }

    {
    constexpr const std::pair<int, const int> p { 1, 2 };
    static_assert(std::get<int>(std::move(p)) == 1, "");
    static_assert(std::get<const int>(std::move(p)) == 2, "");
    }
}
