//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ostream>

// template <class charT, class traits = char_traits<charT> >
//   class basic_ostream;

// operator<<(short n);
// operator<<(unsigned short n);
// operator<<(int n);
// operator<<(unsigned int n);
// operator<<(long n);
// operator<<(unsigned long n);
// operator<<(long long n);
// operator<<(unsigned long long n);

//  Testing to make sure that the max length values are correctly inserted when
//  using std::showbase

// This test exposes a regression that was not fixed yet in the libc++
// shipped with macOS 10.12, 10.13 and 10.14. See D32670 for details.
// XFAIL: with_system_cxx_lib=macosx10.14
// XFAIL: with_system_cxx_lib=macosx10.13
// XFAIL: with_system_cxx_lib=macosx10.12

#include <cassert>
#include <cstdint>
#include <ios>
#include <limits>
#include <sstream>
#include <type_traits>

template <typename T>
static void test(std::ios_base::fmtflags fmt, const char *expected)
{
    std::stringstream ss;
    ss.setf(fmt, std::ios_base::basefield);
    ss << std::showbase << (std::is_signed<T>::value ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max());
    assert(ss.str() == expected);
}

int main()
{
    const std::ios_base::fmtflags o = std::ios_base::oct;
    const std::ios_base::fmtflags d = std::ios_base::dec;
    const std::ios_base::fmtflags x = std::ios_base::hex;

    test<short>(o, "0100000");
    test<short>(d, "-32768");
    test<short>(x, "0x8000");

    test<unsigned short>(o, "0177777");
    test<unsigned short>(d, "65535");
    test<unsigned short>(x, "0xffff");

    test<int>(o, "020000000000");
    test<int>(d, "-2147483648");
    test<int>(x, "0x80000000");

    test<unsigned int>(o, "037777777777");
    test<unsigned int>(d, "4294967295");
    test<unsigned int>(x, "0xffffffff");

    const bool long_is_32 = std::integral_constant<bool, sizeof(long) == sizeof(int32_t)>::value; // avoid compiler warnings
    const bool long_is_64 = std::integral_constant<bool, sizeof(long) == sizeof(int64_t)>::value; // avoid compiler warnings
    const bool long_long_is_64 = std::integral_constant<bool, sizeof(long long) == sizeof(int64_t)>::value; // avoid compiler warnings

    if (long_is_32) {
        test<long>(o, "020000000000");
        test<long>(d, "-2147483648");
        test<long>(x, "0x80000000");

        test<unsigned long>(o, "037777777777");
        test<unsigned long>(d, "4294967295");
        test<unsigned long>(x, "0xffffffff");
    } else if (long_is_64) {
        test<long>(o, "01000000000000000000000");
        test<long>(d, "-9223372036854775808");
        test<long>(x, "0x8000000000000000");

        test<unsigned long>(o, "01777777777777777777777");
        test<unsigned long>(d, "18446744073709551615");
        test<unsigned long>(x, "0xffffffffffffffff");
    }
    if (long_long_is_64) {
        test<long long>(o, "01000000000000000000000");
        test<long long>(d, "-9223372036854775808");
        test<long long>(x, "0x8000000000000000");

        test<unsigned long long>(o, "01777777777777777777777");
        test<unsigned long long>(d, "18446744073709551615");
        test<unsigned long long>(x, "0xffffffffffffffff");
    }

    return 0;
}
