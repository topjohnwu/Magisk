// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8, clang-3.9
// UNSUPPORTED: apple-clang-6, apple-clang-7, apple-clang-8
// Note: libc++ supports string_view before C++17, but literals were introduced in C++14

#include <string_view>
#include <cassert>

int main()
{
    using namespace std;

    string_view foo  =   ""sv;
    assert(foo.length() == 0);
}
