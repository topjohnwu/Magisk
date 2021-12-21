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

// typedef atomic<int_least8_t>   atomic_int_least8_t;
// typedef atomic<uint_least8_t>  atomic_uint_least8_t;
// typedef atomic<int_least16_t>  atomic_int_least16_t;
// typedef atomic<uint_least16_t> atomic_uint_least16_t;
// typedef atomic<int_least32_t>  atomic_int_least32_t;
// typedef atomic<uint_least32_t> atomic_uint_least32_t;
// typedef atomic<int_least64_t>  atomic_int_least64_t;
// typedef atomic<uint_least64_t> atomic_uint_least64_t;
//
// typedef atomic<int_fast8_t>   atomic_int_fast8_t;
// typedef atomic<uint_fast8_t>  atomic_uint_fast8_t;
// typedef atomic<int_fast16_t>  atomic_int_fast16_t;
// typedef atomic<uint_fast16_t> atomic_uint_fast16_t;
// typedef atomic<int_fast32_t>  atomic_int_fast32_t;
// typedef atomic<uint_fast32_t> atomic_uint_fast32_t;
// typedef atomic<int_fast64_t>  atomic_int_fast64_t;
// typedef atomic<uint_fast64_t> atomic_uint_fast64_t;
//
// typedef atomic<intptr_t>  atomic_intptr_t;
// typedef atomic<uintptr_t> atomic_uintptr_t;
// typedef atomic<size_t>    atomic_size_t;
// typedef atomic<ptrdiff_t> atomic_ptrdiff_t;
// typedef atomic<intmax_t>  atomic_intmax_t;
// typedef atomic<uintmax_t> atomic_uintmax_t;

#include <atomic>
#include <type_traits>
#include <cstdint>

int main()
{
    static_assert((std::is_same<std::atomic<  std::int_least8_t>,   std::atomic_int_least8_t>::value), "");
    static_assert((std::is_same<std::atomic< std::uint_least8_t>,  std::atomic_uint_least8_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_least16_t>,  std::atomic_int_least16_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_least16_t>, std::atomic_uint_least16_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_least32_t>,  std::atomic_int_least32_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_least32_t>, std::atomic_uint_least32_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_least64_t>,  std::atomic_int_least64_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_least64_t>, std::atomic_uint_least64_t>::value), "");

    static_assert((std::is_same<std::atomic<  std::int_fast8_t>,   std::atomic_int_fast8_t>::value), "");
    static_assert((std::is_same<std::atomic< std::uint_fast8_t>,  std::atomic_uint_fast8_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_fast16_t>,  std::atomic_int_fast16_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_fast16_t>, std::atomic_uint_fast16_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_fast32_t>,  std::atomic_int_fast32_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_fast32_t>, std::atomic_uint_fast32_t>::value), "");
    static_assert((std::is_same<std::atomic< std::int_fast64_t>,  std::atomic_int_fast64_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uint_fast64_t>, std::atomic_uint_fast64_t>::value), "");

    static_assert((std::is_same<std::atomic< std::intptr_t>,  std::atomic_intptr_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uintptr_t>, std::atomic_uintptr_t>::value), "");
    static_assert((std::is_same<std::atomic<   std::size_t>,    std::atomic_size_t>::value), "");
    static_assert((std::is_same<std::atomic<std::ptrdiff_t>, std::atomic_ptrdiff_t>::value), "");
    static_assert((std::is_same<std::atomic< std::intmax_t>,  std::atomic_intmax_t>::value), "");
    static_assert((std::is_same<std::atomic<std::uintmax_t>, std::atomic_uintmax_t>::value), "");
}
