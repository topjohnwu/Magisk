//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// AppleClang <= 10 enables aligned allocation regardless of the deployment
// target, so this test would fail.
// UNSUPPORTED: apple-clang-9, apple-clang-10

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

#include <new>


#ifdef _LIBCPP_HAS_NO_ALIGNED_ALLOCATION
#   error "libc++ should have aligned allocation in C++17 and up when targeting a platform that supports it"
#endif

int main() { }
