//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Due to C++17 inline variables ASAN flags this test as containing an ODR
// violation because Clock::is_steady is defined in both the dylib and this TU.
// UNSUPPORTED: asan

// Starting with C++17, Clock::is_steady is inlined (but not before LLVM-3.9!),
// but before C++17 it requires the symbol to be present in the dylib, which
// is only shipped starting with macosx10.9.
// XFAIL: with_system_cxx_lib=macosx10.7 && (c++98 || c++03 || c++11 || c++14 || apple-clang-7 || apple-clang-8.0)
// XFAIL: with_system_cxx_lib=macosx10.8 && (c++98 || c++03 || c++11 || c++14 || apple-clang-7 || apple-clang-8.0)

// <chrono>

// high_resolution_clock

// check clock invariants

#include <chrono>

template <class T>
void test(const T &) {}

int main()
{
    typedef std::chrono::high_resolution_clock C;
    static_assert((std::is_same<C::rep, C::duration::rep>::value), "");
    static_assert((std::is_same<C::period, C::duration::period>::value), "");
    static_assert((std::is_same<C::duration, C::time_point::duration>::value), "");
    static_assert(C::is_steady || !C::is_steady, "");
    test(std::chrono::high_resolution_clock::is_steady);
}
