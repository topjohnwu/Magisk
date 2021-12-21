//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// aligned_storage
//
//  Issue 3034 added:
//  The member typedef type shall be a trivial standard-layout type.

#include <type_traits>
#include <cstddef>       // for std::max_align_t
#include "test_macros.h"

int main()
{
    {
    typedef std::aligned_storage<10, 1 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 1>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 1, "");
    static_assert(sizeof(T1) == 10, "");
    }
    {
    typedef std::aligned_storage<10, 2 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 2>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 2, "");
    static_assert(sizeof(T1) == 10, "");
    }
    {
    typedef std::aligned_storage<10, 4 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 4>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 4, "");
    static_assert(sizeof(T1) == 12, "");
    }
    {
    typedef std::aligned_storage<10, 8 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 8>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 8, "");
    static_assert(sizeof(T1) == 16, "");
    }
    {
    typedef std::aligned_storage<10, 16 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 16>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 16, "");
    static_assert(sizeof(T1) == 16, "");
    }
    {
    typedef std::aligned_storage<10, 32 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10, 32>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 32, "");
    static_assert(sizeof(T1) == 32, "");
    }
    {
    typedef std::aligned_storage<20, 32 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<20, 32>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 32, "");
    static_assert(sizeof(T1) == 32, "");
    }
    {
    typedef std::aligned_storage<40, 32 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<40, 32>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 32, "");
    static_assert(sizeof(T1) == 64, "");
    }
    {
    typedef std::aligned_storage<12, 16 >::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<12, 16>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 16, "");
    static_assert(sizeof(T1) == 16, "");
    }
    {
    typedef std::aligned_storage<1>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<1>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 1, "");
    static_assert(sizeof(T1) == 1, "");
    }
    {
    typedef std::aligned_storage<2>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<2>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 2, "");
    static_assert(sizeof(T1) == 2, "");
    }
    {
    typedef std::aligned_storage<3>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<3>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 2, "");
    static_assert(sizeof(T1) == 4, "");
    }
    {
    typedef std::aligned_storage<4>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<4>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 4, "");
    static_assert(sizeof(T1) == 4, "");
    }
    {
    typedef std::aligned_storage<5>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<5>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 4, "");
    static_assert(sizeof(T1) == 8, "");
    }
    {
    typedef std::aligned_storage<7>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<7>, T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 4, "");
    static_assert(sizeof(T1) == 8, "");
    }
    {
    typedef std::aligned_storage<8>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<8>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 8, "");
    static_assert(sizeof(T1) == 8, "");
    }
    {
    typedef std::aligned_storage<9>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<9>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 8, "");
    static_assert(sizeof(T1) == 16, "");
    }
    {
    typedef std::aligned_storage<15>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<15>, T1>::value, "");
#endif
#if TEST_STD_VER <= 17
    static_assert(std::is_pod<T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 8, "");
    static_assert(sizeof(T1) == 16, "");
    }
    // Use alignof(std::max_align_t) below to find the max alignment instead of
    // hardcoding it, because it's different on different platforms.
    // (For example 8 on arm and 16 on x86.)
#if TEST_STD_VER < 11
#define alignof __alignof__
#endif
    {
    typedef std::aligned_storage<16>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<16>, T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == alignof(std::max_align_t),
                  "");
    static_assert(sizeof(T1) == 16, "");
    }
    {
    typedef std::aligned_storage<17>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<17>, T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == alignof(std::max_align_t),
                  "");
    static_assert(sizeof(T1) == 16 + alignof(std::max_align_t), "");
    }
    {
    typedef std::aligned_storage<10>::type T1;
#if TEST_STD_VER > 11
    static_assert(std::is_same<std::aligned_storage_t<10>, T1>::value, "");
#endif
    static_assert(std::is_trivial<T1>::value, "");
    static_assert(std::is_standard_layout<T1>::value, "");
    static_assert(std::alignment_of<T1>::value == 8, "");
    static_assert(sizeof(T1) == 16, "");
    }
}
