//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class strstream

// strstream();

#include <strstream>
#include <cassert>
#include <string>

int main()
{
    std::strstream inout;
    int i = 123;
    double d = 4.5;
    std::string s("dog");
    inout << i << ' ' << d << ' ' << s << std::ends;
    assert(inout.str() == std::string("123 4.5 dog"));
    i = 0;
    d = 0;
    s = "";
    inout >> i >> d >> s;
    assert(i == 123);
    assert(d == 4.5);
    assert(strcmp(s.c_str(), "dog") == 0);
    inout.freeze(false);
}
