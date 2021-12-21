//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// XFAIL: with_system_cxx_lib=macosx10.14
// XFAIL: with_system_cxx_lib=macosx10.13
// XFAIL: with_system_cxx_lib=macosx10.12
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9
// XFAIL: with_system_cxx_lib=macosx10.8
// XFAIL: with_system_cxx_lib=macosx10.7

// <charconv>

// from_chars_result from_chars(const char* first, const char* last,
//                              Integral& value, int base = 10)

#include "charconv_test_helpers.h"

template <typename T>
struct test_basics : roundtrip_test_base<T>
{
    using roundtrip_test_base<T>::test;

    void operator()()
    {
        test(0);
        test(42);
        test(32768);
        test(0, 10);
        test(42, 10);
        test(32768, 10);
        test(0xf, 16);
        test(0xdeadbeaf, 16);
        test(0755, 8);

        for (int b = 2; b < 37; ++b)
        {
            using xl = std::numeric_limits<T>;

            test(1, b);
            test(-1, b);
            test(xl::lowest(), b);
            test((xl::max)(), b);
            test((xl::max)() / 2, b);
        }

        using std::from_chars;
        std::from_chars_result r;
        T x;

        {
            char s[] = "001x";

            // the expected form of the subject sequence is a sequence of
            // letters and digits representing an integer with the radix
            // specified by base (C11 7.22.1.4/3)
            r = from_chars(s, s + sizeof(s), x);
            assert(r.ec == std::errc{});
            assert(r.ptr == s + 3);
            assert(x == 1);
        }

        {
            char s[] = "0X7BAtSGHDkEIXZg ";

            // The letters from a (or A) through z (or Z) are ascribed the
            // values 10 through 35; (C11 7.22.1.4/3)
            r = from_chars(s, s + sizeof(s), x, 36);
            assert(r.ec == std::errc::result_out_of_range);
            // The member ptr of the return value points to the first character
            // not matching the pattern
            assert(r.ptr == s + sizeof(s) - 2);
            assert(x == 1);

            // no "0x" or "0X" prefix shall appear if the value of base is 16
            r = from_chars(s, s + sizeof(s), x, 16);
            assert(r.ec == std::errc{});
            assert(r.ptr == s + 1);
            assert(x == 0);

            // only letters and digits whose ascribed values are less than that
            // of base are permitted. (C11 7.22.1.4/3)
            r = from_chars(s + 2, s + sizeof(s), x, 12);
            // If the parsed value is not in the range representable by the type
            // of value,
            if (!fits_in<T>(1150))
            {
                // value is unmodified and
                assert(x == 0);
                // the member ec of the return value is equal to
                // errc::result_out_of_range
                assert(r.ec == std::errc::result_out_of_range);
            }
            else
            {
                // Otherwise, value is set to the parsed value,
                assert(x == 1150);
                // and the member ec is value-initialized.
                assert(r.ec == std::errc{});
            }
            assert(r.ptr == s + 5);
        }
    }
};

template <typename T>
struct test_signed : roundtrip_test_base<T>
{
    using roundtrip_test_base<T>::test;

    void operator()()
    {
        test(-1);
        test(-12);
        test(-1, 10);
        test(-12, 10);
        test(-21734634, 10);
        test(-2647, 2);
        test(-0xcc1, 16);

        for (int b = 2; b < 37; ++b)
        {
            using xl = std::numeric_limits<T>;

            test(0, b);
            test(xl::lowest(), b);
            test((xl::max)(), b);
        }

        using std::from_chars;
        std::from_chars_result r;
        T x;

        {
            // If the pattern allows for an optional sign,
            // but the string has no digit characters following the sign,
            char s[] = "- 9+12";
            r = from_chars(s, s + sizeof(s), x);
            // no characters match the pattern.
            assert(r.ptr == s);
            assert(r.ec == std::errc::invalid_argument);
        }

        {
            char s[] = "9+12";
            r = from_chars(s, s + sizeof(s), x);
            assert(r.ec == std::errc{});
            // The member ptr of the return value points to the first character
            // not matching the pattern,
            assert(r.ptr == s + 1);
            assert(x == 9);
        }

        {
            char s[] = "12";
            r = from_chars(s, s + 2, x);
            assert(r.ec == std::errc{});
            // or has the value last if all characters match.
            assert(r.ptr == s + 2);
            assert(x == 12);
        }

        {
            // '-' is the only sign that may appear
            char s[] = "+30";
            // If no characters match the pattern,
            r = from_chars(s, s + sizeof(s), x);
            // value is unmodified,
            assert(x == 12);
            // the member ptr of the return value is first and
            assert(r.ptr == s);
            // the member ec is equal to errc::invalid_argument.
            assert(r.ec == std::errc::invalid_argument);
        }
    }
};

int main()
{
    run<test_basics>(integrals);
    run<test_signed>(all_signed);
}
