//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <atomic>

// typedef atomic<char>               atomic_char;
// typedef atomic<signed char>        atomic_schar;
// typedef atomic<unsigned char>      atomic_uchar;
// typedef atomic<short>              atomic_short;
// typedef atomic<unsigned short>     atomic_ushort;
// typedef atomic<int>                atomic_int;
// typedef atomic<unsigned int>       atomic_uint;
// typedef atomic<long>               atomic_long;
// typedef atomic<unsigned long>      atomic_ulong;
// typedef atomic<long long>          atomic_llong;
// typedef atomic<unsigned long long> atomic_ullong;
// typedef atomic<char16_t>           atomic_char16_t;
// typedef atomic<char32_t>           atomic_char32_t;
// typedef atomic<wchar_t>            atomic_wchar_t;
//
// typedef atomic<intptr_t>           atomic_intptr_t;
// typedef atomic<uintptr_t>          atomic_uintptr_t;
//
// typedef atomic<int8_t>             atomic_int8_t;
// typedef atomic<uint8_t>            atomic_uint8_t;
// typedef atomic<int16_t>            atomic_int16_t;
// typedef atomic<uint16_t>           atomic_uint16_t;
// typedef atomic<int32_t>            atomic_int32_t;
// typedef atomic<uint32_t>           atomic_uint32_t;
// typedef atomic<int64_t>            atomic_int64_t;
// typedef atomic<uint64_t>           atomic_uint64_t;

#include <atomic>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::atomic<char>, std::atomic_char>::value), "");
    static_assert((std::is_same<std::atomic<signed char>, std::atomic_schar>::value), "");
    static_assert((std::is_same<std::atomic<unsigned char>, std::atomic_uchar>::value), "");
    static_assert((std::is_same<std::atomic<short>, std::atomic_short>::value), "");
    static_assert((std::is_same<std::atomic<unsigned short>, std::atomic_ushort>::value), "");
    static_assert((std::is_same<std::atomic<int>, std::atomic_int>::value), "");
    static_assert((std::is_same<std::atomic<unsigned int>, std::atomic_uint>::value), "");
    static_assert((std::is_same<std::atomic<long>, std::atomic_long>::value), "");
    static_assert((std::is_same<std::atomic<unsigned long>, std::atomic_ulong>::value), "");
    static_assert((std::is_same<std::atomic<long long>, std::atomic_llong>::value), "");
    static_assert((std::is_same<std::atomic<unsigned long long>, std::atomic_ullong>::value), "");
    static_assert((std::is_same<std::atomic<wchar_t>, std::atomic_wchar_t>::value), "");
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    static_assert((std::is_same<std::atomic<char16_t>, std::atomic_char16_t>::value), "");
    static_assert((std::is_same<std::atomic<char32_t>, std::atomic_char32_t>::value), "");
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS

//  Added by LWG 2441
    static_assert((std::is_same<std::atomic<intptr_t>,  std::atomic_intptr_t>::value), "");
    static_assert((std::is_same<std::atomic<uintptr_t>, std::atomic_uintptr_t>::value), "");

    static_assert((std::is_same<std::atomic<int8_t>,    std::atomic_int8_t>::value), "");
    static_assert((std::is_same<std::atomic<uint8_t>,   std::atomic_uint8_t>::value), "");
    static_assert((std::is_same<std::atomic<int16_t>,   std::atomic_int16_t>::value), "");
    static_assert((std::is_same<std::atomic<uint16_t>,  std::atomic_uint16_t>::value), "");
    static_assert((std::is_same<std::atomic<int32_t>,   std::atomic_int32_t>::value), "");
    static_assert((std::is_same<std::atomic<uint32_t>,  std::atomic_uint32_t>::value), "");
    static_assert((std::is_same<std::atomic<int64_t>,   std::atomic_int64_t>::value), "");
    static_assert((std::is_same<std::atomic<uint64_t>,  std::atomic_uint64_t>::value), "");
}
