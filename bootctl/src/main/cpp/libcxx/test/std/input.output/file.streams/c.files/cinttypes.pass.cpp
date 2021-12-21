//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <cinttypes>

#include <cinttypes>
#include <type_traits>

#ifndef INT8_MIN
#error INT8_MIN not defined
#endif

#ifndef INT16_MIN
#error INT16_MIN not defined
#endif

#ifndef INT32_MIN
#error INT32_MIN not defined
#endif

#ifndef INT64_MIN
#error INT64_MIN not defined
#endif

#ifndef INT8_MAX
#error INT8_MAX not defined
#endif

#ifndef INT16_MAX
#error INT16_MAX not defined
#endif

#ifndef INT32_MAX
#error INT32_MAX not defined
#endif

#ifndef INT64_MAX
#error INT64_MAX not defined
#endif

#ifndef UINT8_MAX
#error UINT8_MAX not defined
#endif

#ifndef UINT16_MAX
#error UINT16_MAX not defined
#endif

#ifndef UINT32_MAX
#error UINT32_MAX not defined
#endif

#ifndef UINT64_MAX
#error UINT64_MAX not defined
#endif

#ifndef INT_LEAST8_MIN
#error INT_LEAST8_MIN not defined
#endif

#ifndef INT_LEAST16_MIN
#error INT_LEAST16_MIN not defined
#endif

#ifndef INT_LEAST32_MIN
#error INT_LEAST32_MIN not defined
#endif

#ifndef INT_LEAST64_MIN
#error INT_LEAST64_MIN not defined
#endif

#ifndef INT_LEAST8_MAX
#error INT_LEAST8_MAX not defined
#endif

#ifndef INT_LEAST16_MAX
#error INT_LEAST16_MAX not defined
#endif

#ifndef INT_LEAST32_MAX
#error INT_LEAST32_MAX not defined
#endif

#ifndef INT_LEAST64_MAX
#error INT_LEAST64_MAX not defined
#endif

#ifndef UINT_LEAST8_MAX
#error UINT_LEAST8_MAX not defined
#endif

#ifndef UINT_LEAST16_MAX
#error UINT_LEAST16_MAX not defined
#endif

#ifndef UINT_LEAST32_MAX
#error UINT_LEAST32_MAX not defined
#endif

#ifndef UINT_LEAST64_MAX
#error UINT_LEAST64_MAX not defined
#endif

#ifndef INT_FAST8_MIN
#error INT_FAST8_MIN not defined
#endif

#ifndef INT_FAST16_MIN
#error INT_FAST16_MIN not defined
#endif

#ifndef INT_FAST32_MIN
#error INT_FAST32_MIN not defined
#endif

#ifndef INT_FAST64_MIN
#error INT_FAST64_MIN not defined
#endif

#ifndef INT_FAST8_MAX
#error INT_FAST8_MAX not defined
#endif

#ifndef INT_FAST16_MAX
#error INT_FAST16_MAX not defined
#endif

#ifndef INT_FAST32_MAX
#error INT_FAST32_MAX not defined
#endif

#ifndef INT_FAST64_MAX
#error INT_FAST64_MAX not defined
#endif

#ifndef UINT_FAST8_MAX
#error UINT_FAST8_MAX not defined
#endif

#ifndef UINT_FAST16_MAX
#error UINT_FAST16_MAX not defined
#endif

#ifndef UINT_FAST32_MAX
#error UINT_FAST32_MAX not defined
#endif

#ifndef UINT_FAST64_MAX
#error UINT_FAST64_MAX not defined
#endif

#ifndef INTPTR_MIN
#error INTPTR_MIN not defined
#endif

#ifndef INTPTR_MAX
#error INTPTR_MAX not defined
#endif

#ifndef UINTPTR_MAX
#error UINTPTR_MAX not defined
#endif

#ifndef INTMAX_MIN
#error INTMAX_MIN not defined
#endif

#ifndef INTMAX_MAX
#error INTMAX_MAX not defined
#endif

#ifndef UINTMAX_MAX
#error UINTMAX_MAX not defined
#endif

#ifndef PTRDIFF_MIN
#error PTRDIFF_MIN not defined
#endif

#ifndef PTRDIFF_MAX
#error PTRDIFF_MAX not defined
#endif

#ifndef SIG_ATOMIC_MIN
#error SIG_ATOMIC_MIN not defined
#endif

#ifndef SIG_ATOMIC_MAX
#error SIG_ATOMIC_MAX not defined
#endif

#ifndef SIZE_MAX
#error SIZE_MAX not defined
#endif

#ifndef WCHAR_MIN
#error WCHAR_MIN not defined
#endif

#ifndef WCHAR_MAX
#error WCHAR_MAX not defined
#endif

#ifndef WINT_MIN
#error WINT_MIN not defined
#endif

#ifndef WINT_MAX
#error WINT_MAX not defined
#endif

#ifndef INT8_C
#error INT8_C not defined
#endif

#ifndef INT16_C
#error INT16_C not defined
#endif

#ifndef INT32_C
#error INT32_C not defined
#endif

#ifndef INT64_C
#error INT64_C not defined
#endif

#ifndef UINT8_C
#error UINT8_C not defined
#endif

#ifndef UINT16_C
#error UINT16_C not defined
#endif

#ifndef UINT32_C
#error UINT32_C not defined
#endif

#ifndef UINT64_C
#error UINT64_C not defined
#endif

#ifndef INTMAX_C
#error INTMAX_C not defined
#endif

#ifndef UINTMAX_C
#error UINTMAX_C not defined
#endif

#ifndef PRId8
#error PRId8 not defined
#endif

#ifndef PRId16
#error PRId16 not defined
#endif

#ifndef PRId32
#error PRId32 not defined
#endif

#ifndef PRId64
#error PRId64 not defined
#endif

#ifndef PRIdLEAST8
#error PRIdLEAST8 not defined
#endif

#ifndef PRIdLEAST16
#error PRIdLEAST16 not defined
#endif

#ifndef PRIdLEAST32
#error PRIdLEAST32 not defined
#endif

#ifndef PRIdLEAST64
#error PRIdLEAST64 not defined
#endif

#ifndef PRIdFAST8
#error PRIdFAST8 not defined
#endif

#ifndef PRIdFAST16
#error PRIdFAST16 not defined
#endif

#ifndef PRIdFAST32
#error PRIdFAST32 not defined
#endif

#ifndef PRIdFAST64
#error PRIdFAST64 not defined
#endif

#ifndef PRIdMAX
#error PRIdMAX not defined
#endif

#ifndef PRIdPTR
#error PRIdPTR not defined
#endif

#ifndef PRIi8
#error PRIi8 not defined
#endif

#ifndef PRIi16
#error PRIi16 not defined
#endif

#ifndef PRIi32
#error PRIi32 not defined
#endif

#ifndef PRIi64
#error PRIi64 not defined
#endif

#ifndef PRIiLEAST8
#error PRIiLEAST8 not defined
#endif

#ifndef PRIiLEAST16
#error PRIiLEAST16 not defined
#endif

#ifndef PRIiLEAST32
#error PRIiLEAST32 not defined
#endif

#ifndef PRIiLEAST64
#error PRIiLEAST64 not defined
#endif

#ifndef PRIiFAST8
#error PRIiFAST8 not defined
#endif

#ifndef PRIiFAST16
#error PRIiFAST16 not defined
#endif

#ifndef PRIiFAST32
#error PRIiFAST32 not defined
#endif

#ifndef PRIiFAST64
#error PRIiFAST64 not defined
#endif

#ifndef PRIiMAX
#error PRIiMAX not defined
#endif

#ifndef PRIiPTR
#error PRIiPTR not defined
#endif

#ifndef PRIo8
#error PRIo8 not defined
#endif

#ifndef PRIo16
#error PRIo16 not defined
#endif

#ifndef PRIo32
#error PRIo32 not defined
#endif

#ifndef PRIo64
#error PRIo64 not defined
#endif

#ifndef PRIoLEAST8
#error PRIoLEAST8 not defined
#endif

#ifndef PRIoLEAST16
#error PRIoLEAST16 not defined
#endif

#ifndef PRIoLEAST32
#error PRIoLEAST32 not defined
#endif

#ifndef PRIoLEAST64
#error PRIoLEAST64 not defined
#endif

#ifndef PRIoFAST8
#error PRIoFAST8 not defined
#endif

#ifndef PRIoFAST16
#error PRIoFAST16 not defined
#endif

#ifndef PRIoFAST32
#error PRIoFAST32 not defined
#endif

#ifndef PRIoFAST64
#error PRIoFAST64 not defined
#endif

#ifndef PRIoMAX
#error PRIoMAX not defined
#endif

#ifndef PRIoPTR
#error PRIoPTR not defined
#endif

#ifndef PRIu8
#error PRIu8 not defined
#endif

#ifndef PRIu16
#error PRIu16 not defined
#endif

#ifndef PRIu32
#error PRIu32 not defined
#endif

#ifndef PRIu64
#error PRIu64 not defined
#endif

#ifndef PRIuLEAST8
#error PRIuLEAST8 not defined
#endif

#ifndef PRIuLEAST16
#error PRIuLEAST16 not defined
#endif

#ifndef PRIuLEAST32
#error PRIuLEAST32 not defined
#endif

#ifndef PRIuLEAST64
#error PRIuLEAST64 not defined
#endif

#ifndef PRIuFAST8
#error PRIuFAST8 not defined
#endif

#ifndef PRIuFAST16
#error PRIuFAST16 not defined
#endif

#ifndef PRIuFAST32
#error PRIuFAST32 not defined
#endif

#ifndef PRIuFAST64
#error PRIuFAST64 not defined
#endif

#ifndef PRIuMAX
#error PRIuMAX not defined
#endif

#ifndef PRIuPTR
#error PRIuPTR not defined
#endif

#ifndef PRIx8
#error PRIx8 not defined
#endif

#ifndef PRIx16
#error PRIx16 not defined
#endif

#ifndef PRIx32
#error PRIx32 not defined
#endif

#ifndef PRIx64
#error PRIx64 not defined
#endif

#ifndef PRIxLEAST8
#error PRIxLEAST8 not defined
#endif

#ifndef PRIxLEAST16
#error PRIxLEAST16 not defined
#endif

#ifndef PRIxLEAST32
#error PRIxLEAST32 not defined
#endif

#ifndef PRIxLEAST64
#error PRIxLEAST64 not defined
#endif

#ifndef PRIxFAST8
#error PRIxFAST8 not defined
#endif

#ifndef PRIxFAST16
#error PRIxFAST16 not defined
#endif

#ifndef PRIxFAST32
#error PRIxFAST32 not defined
#endif

#ifndef PRIxFAST64
#error PRIxFAST64 not defined
#endif

#ifndef PRIxMAX
#error PRIxMAX not defined
#endif

#ifndef PRIxPTR
#error PRIxPTR not defined
#endif

#ifndef PRIX8
#error PRIX8 not defined
#endif

#ifndef PRIX16
#error PRIX16 not defined
#endif

#ifndef PRIX32
#error PRIX32 not defined
#endif

#ifndef PRIX64
#error PRIX64 not defined
#endif

#ifndef PRIXLEAST8
#error PRIXLEAST8 not defined
#endif

#ifndef PRIXLEAST16
#error PRIXLEAST16 not defined
#endif

#ifndef PRIXLEAST32
#error PRIXLEAST32 not defined
#endif

#ifndef PRIXLEAST64
#error PRIXLEAST64 not defined
#endif

#ifndef PRIXFAST8
#error PRIXFAST8 not defined
#endif

#ifndef PRIXFAST16
#error PRIXFAST16 not defined
#endif

#ifndef PRIXFAST32
#error PRIXFAST32 not defined
#endif

#ifndef PRIXFAST64
#error PRIXFAST64 not defined
#endif

#ifndef PRIXMAX
#error PRIXMAX not defined
#endif

#ifndef PRIXPTR
#error PRIXPTR not defined
#endif

#ifndef SCNd8
#error SCNd8 not defined
#endif

#ifndef SCNd16
#error SCNd16 not defined
#endif

#ifndef SCNd32
#error SCNd32 not defined
#endif

#ifndef SCNd64
#error SCNd64 not defined
#endif

#ifndef SCNdLEAST8
#error SCNdLEAST8 not defined
#endif

#ifndef SCNdLEAST16
#error SCNdLEAST16 not defined
#endif

#ifndef SCNdLEAST32
#error SCNdLEAST32 not defined
#endif

#ifndef SCNdLEAST64
#error SCNdLEAST64 not defined
#endif

#ifndef SCNdFAST8
#error SCNdFAST8 not defined
#endif

#ifndef SCNdFAST16
#error SCNdFAST16 not defined
#endif

#ifndef SCNdFAST32
#error SCNdFAST32 not defined
#endif

#ifndef SCNdFAST64
#error SCNdFAST64 not defined
#endif

#ifndef SCNdMAX
#error SCNdMAX not defined
#endif

#ifndef SCNdPTR
#error SCNdPTR not defined
#endif

#ifndef SCNi8
#error SCNi8 not defined
#endif

#ifndef SCNi16
#error SCNi16 not defined
#endif

#ifndef SCNi32
#error SCNi32 not defined
#endif

#ifndef SCNi64
#error SCNi64 not defined
#endif

#ifndef SCNiLEAST8
#error SCNiLEAST8 not defined
#endif

#ifndef SCNiLEAST16
#error SCNiLEAST16 not defined
#endif

#ifndef SCNiLEAST32
#error SCNiLEAST32 not defined
#endif

#ifndef SCNiLEAST64
#error SCNiLEAST64 not defined
#endif

#ifndef SCNiFAST8
#error SCNiFAST8 not defined
#endif

#ifndef SCNiFAST16
#error SCNiFAST16 not defined
#endif

#ifndef SCNiFAST32
#error SCNiFAST32 not defined
#endif

#ifndef SCNiFAST64
#error SCNiFAST64 not defined
#endif

#ifndef SCNiMAX
#error SCNiMAX not defined
#endif

#ifndef SCNiPTR
#error SCNiPTR not defined
#endif

#ifndef SCNo8
#error SCNo8 not defined
#endif

#ifndef SCNo16
#error SCNo16 not defined
#endif

#ifndef SCNo32
#error SCNo32 not defined
#endif

#ifndef SCNo64
#error SCNo64 not defined
#endif

#ifndef SCNoLEAST8
#error SCNoLEAST8 not defined
#endif

#ifndef SCNoLEAST16
#error SCNoLEAST16 not defined
#endif

#ifndef SCNoLEAST32
#error SCNoLEAST32 not defined
#endif

#ifndef SCNoLEAST64
#error SCNoLEAST64 not defined
#endif

#ifndef SCNoFAST8
#error SCNoFAST8 not defined
#endif

#ifndef SCNoFAST16
#error SCNoFAST16 not defined
#endif

#ifndef SCNoFAST32
#error SCNoFAST32 not defined
#endif

#ifndef SCNoFAST64
#error SCNoFAST64 not defined
#endif

#ifndef SCNoMAX
#error SCNoMAX not defined
#endif

#ifndef SCNoPTR
#error SCNoPTR not defined
#endif

#ifndef SCNu8
#error SCNu8 not defined
#endif

#ifndef SCNu16
#error SCNu16 not defined
#endif

#ifndef SCNu32
#error SCNu32 not defined
#endif

#ifndef SCNu64
#error SCNu64 not defined
#endif

#ifndef SCNuLEAST8
#error SCNuLEAST8 not defined
#endif

#ifndef SCNuLEAST16
#error SCNuLEAST16 not defined
#endif

#ifndef SCNuLEAST32
#error SCNuLEAST32 not defined
#endif

#ifndef SCNuLEAST64
#error SCNuLEAST64 not defined
#endif

#ifndef SCNuFAST8
#error SCNuFAST8 not defined
#endif

#ifndef SCNuFAST16
#error SCNuFAST16 not defined
#endif

#ifndef SCNuFAST32
#error SCNuFAST32 not defined
#endif

#ifndef SCNuFAST64
#error SCNuFAST64 not defined
#endif

#ifndef SCNuMAX
#error SCNuMAX not defined
#endif

#ifndef SCNuPTR
#error SCNuPTR not defined
#endif

#ifndef SCNx8
#error SCNx8 not defined
#endif

#ifndef SCNx16
#error SCNx16 not defined
#endif

#ifndef SCNx32
#error SCNx32 not defined
#endif

#ifndef SCNx64
#error SCNx64 not defined
#endif

#ifndef SCNxLEAST8
#error SCNxLEAST8 not defined
#endif

#ifndef SCNxLEAST16
#error SCNxLEAST16 not defined
#endif

#ifndef SCNxLEAST32
#error SCNxLEAST32 not defined
#endif

#ifndef SCNxLEAST64
#error SCNxLEAST64 not defined
#endif

#ifndef SCNxFAST8
#error SCNxFAST8 not defined
#endif

#ifndef SCNxFAST16
#error SCNxFAST16 not defined
#endif

#ifndef SCNxFAST32
#error SCNxFAST32 not defined
#endif

#ifndef SCNxFAST64
#error SCNxFAST64 not defined
#endif

#ifndef SCNxMAX
#error SCNxMAX not defined
#endif

#ifndef SCNxPTR
#error SCNxPTR not defined
#endif

template <class T> void test()
{
    T t = 0;
    ((void)t); // Prevent unused warning
}

int main()
{
    test<std::int8_t >();
    test<std::int16_t>();
    test<std::int32_t>();
    test<std::int64_t>();

    test<std::uint8_t >();
    test<std::uint16_t>();
    test<std::uint32_t>();
    test<std::uint64_t>();

    test<std::int_least8_t >();
    test<std::int_least16_t>();
    test<std::int_least32_t>();
    test<std::int_least64_t>();

    test<std::uint_least8_t >();
    test<std::uint_least16_t>();
    test<std::uint_least32_t>();
    test<std::uint_least64_t>();

    test<std::int_fast8_t >();
    test<std::int_fast16_t>();
    test<std::int_fast32_t>();
    test<std::int_fast64_t>();

    test<std::uint_fast8_t >();
    test<std::uint_fast16_t>();
    test<std::uint_fast32_t>();
    test<std::uint_fast64_t>();

    test<std::intptr_t >();
    test<std::uintptr_t>();
    test<std::intmax_t >();
    test<std::uintmax_t>();

    {
    std::imaxdiv_t  i1 = {};
    ((void)i1); // Prevent unused warning
    }

    std::intmax_t i = 0;
    ((void)i); // Prevent unused warning
    static_assert((std::is_same<decltype(std::imaxabs(i)), std::intmax_t>::value), "");
    static_assert((std::is_same<decltype(std::imaxdiv(i, i)), std::imaxdiv_t>::value), "");
    static_assert((std::is_same<decltype(std::strtoimax("", (char**)0, 0)), std::intmax_t>::value), "");
    static_assert((std::is_same<decltype(std::strtoumax("", (char**)0, 0)), std::uintmax_t>::value), "");
    static_assert((std::is_same<decltype(std::wcstoimax(L"", (wchar_t**)0, 0)), std::intmax_t>::value), "");
    static_assert((std::is_same<decltype(std::wcstoumax(L"", (wchar_t**)0, 0)), std::uintmax_t>::value), "");
}
