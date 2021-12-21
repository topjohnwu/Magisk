//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// explicit basic_string(basic_string_view<CharT, traits> sv, const Allocator& a = Allocator());

#include <string>
#include <string_view>

void foo ( const string &s ) {}

int main()
{
    std::string_view sv = "ABCDE";
    foo(sv);    // requires implicit conversion from string_view to string
}
