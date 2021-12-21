//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// These constructors are still unavailable in C++03, but this test depends
// on access control SFINAE and fails without it.
// UNSUPPORTED: c++98, c++03

// <locale>

// wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>

// wstring_convert(wstring_convert const&) = delete;
// wstring_convert& operator=(wstring_convert const&) = delete;

#include <locale>
#include <codecvt>
#include <cassert>

int main()
{
    typedef std::codecvt_utf8<wchar_t> Codecvt;
    typedef std::wstring_convert<Codecvt> Myconv;
    static_assert(!std::is_copy_constructible<Myconv>::value, "");
    static_assert(!std::is_copy_assignable<Myconv>::value, "");
}
