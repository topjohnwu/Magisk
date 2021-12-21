//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SUPPORT_CHARCONV_TEST_HELPERS_H
#define SUPPORT_CHARCONV_TEST_HELPERS_H

#include <charconv>
#include <cassert>
#include <limits>
#include <string.h>
#include <stdlib.h>

#include "test_macros.h"

#if TEST_STD_VER <= 11
#error This file requires C++14
#endif

using std::false_type;
using std::true_type;

template <typename To, typename From>
constexpr auto
is_non_narrowing(From a) -> decltype(To{a}, true_type())
{
    return {};
}

template <typename To>
constexpr auto
is_non_narrowing(...) -> false_type
{
    return {};
}

template <typename X, typename T>
constexpr bool
_fits_in(T, true_type /* non-narrowing*/, ...)
{
    return true;
}

template <typename X, typename T, typename xl = std::numeric_limits<X>>
constexpr bool
_fits_in(T v, false_type, true_type /* T signed*/, true_type /* X signed */)
{
    return xl::lowest() <= v && v <= (xl::max)();
}

template <typename X, typename T, typename xl = std::numeric_limits<X>>
constexpr bool
_fits_in(T v, false_type, true_type /* T signed */, false_type /* X unsigned*/)
{
    return 0 <= v && std::make_unsigned_t<T>(v) <= (xl::max)();
}

template <typename X, typename T, typename xl = std::numeric_limits<X>>
constexpr bool
_fits_in(T v, false_type, false_type /* T unsigned */, ...)
{
    return v <= std::make_unsigned_t<X>((xl::max)());
}

template <typename X, typename T>
constexpr bool
fits_in(T v)
{
    return _fits_in<X>(v, is_non_narrowing<X>(v), std::is_signed<T>(),
                       std::is_signed<X>());
}

template <typename X>
struct to_chars_test_base
{
    template <typename T, size_t N, typename... Ts>
    void test(T v, char const (&expect)[N], Ts... args)
    {
        using std::to_chars;
        std::to_chars_result r;

        constexpr size_t len = N - 1;
        static_assert(len > 0, "expected output won't be empty");

        if (!fits_in<X>(v))
            return;

        r = to_chars(buf, buf + len - 1, X(v), args...);
        assert(r.ptr == buf + len - 1);
        assert(r.ec == std::errc::value_too_large);

        r = to_chars(buf, buf + sizeof(buf), X(v), args...);
        assert(r.ptr == buf + len);
        assert(r.ec == std::errc{});
        assert(memcmp(buf, expect, len) == 0);
    }

    template <typename... Ts>
    void test_value(X v, Ts... args)
    {
        using std::to_chars;
        std::to_chars_result r;

        r = to_chars(buf, buf + sizeof(buf), v, args...);
        assert(r.ec == std::errc{});
        *r.ptr = '\0';

        auto a = fromchars(buf, r.ptr, args...);
        assert(v == a);

        auto ep = r.ptr - 1;
        r = to_chars(buf, ep, v, args...);
        assert(r.ptr == ep);
        assert(r.ec == std::errc::value_too_large);
    }

private:
    static auto fromchars(char const* p, char const* ep, int base, true_type)
    {
        char* last;
        auto r = strtoll(p, &last, base);
        assert(last == ep);

        return r;
    }

    static auto fromchars(char const* p, char const* ep, int base, false_type)
    {
        char* last;
        auto r = strtoull(p, &last, base);
        assert(last == ep);

        return r;
    }

    static auto fromchars(char const* p, char const* ep, int base = 10)
    {
        return fromchars(p, ep, base, std::is_signed<X>());
    }

    char buf[100];
};

template <typename X>
struct roundtrip_test_base
{
    template <typename T, typename... Ts>
    void test(T v, Ts... args)
    {
        using std::from_chars;
        using std::to_chars;
        std::from_chars_result r2;
        std::to_chars_result r;
        X x = 0xc;

        if (fits_in<X>(v))
        {
            r = to_chars(buf, buf + sizeof(buf), v, args...);
            assert(r.ec == std::errc{});

            r2 = from_chars(buf, r.ptr, x, args...);
            assert(r2.ptr == r.ptr);
            assert(x == X(v));
        }
        else
        {
            r = to_chars(buf, buf + sizeof(buf), v, args...);
            assert(r.ec == std::errc{});

            r2 = from_chars(buf, r.ptr, x, args...);

            if (std::is_signed<T>::value && v < 0 && std::is_unsigned<X>::value)
            {
                assert(x == 0xc);
                assert(r2.ptr == buf);
                assert(r2.ec == std::errc::invalid_argument);
            }
            else
            {
                assert(x == 0xc);
                assert(r2.ptr == r.ptr);
                assert(r2.ec == std::errc::result_out_of_range);
            }
        }
    }

private:
    char buf[100];
};

template <typename... T>
struct type_list
{
};

template <typename L1, typename L2>
struct type_concat;

template <typename... Xs, typename... Ys>
struct type_concat<type_list<Xs...>, type_list<Ys...>>
{
    using type = type_list<Xs..., Ys...>;
};

template <typename L1, typename L2>
using concat_t = typename type_concat<L1, L2>::type;

template <typename L1, typename L2>
constexpr auto concat(L1, L2) -> concat_t<L1, L2>
{
    return {};
}

auto all_signed = type_list<char, signed char, short, int, long, long long>();
auto all_unsigned = type_list<unsigned char, unsigned short, unsigned int,
                              unsigned long, unsigned long long>();
auto integrals = concat(all_signed, all_unsigned);

template <template <typename> class Fn, typename... Ts>
void
run(type_list<Ts...>)
{
    int ls[sizeof...(Ts)] = {(Fn<Ts>{}(), 0)...};
    (void)ls;
}

#endif  // SUPPORT_CHARCONV_TEST_HELPERS_H
