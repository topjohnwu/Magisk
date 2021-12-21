//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class ostrstream

// ostrstream();

#include <strstream>
#include <cassert>
#include <string>

int main()
{
    std::ostrstream out;
    int i = 123;
    double d = 4.5;
    std::string s("dog");
    out << i << ' ' << d << ' ' << s << std::ends;
    assert(out.str() == std::string("123 4.5 dog"));
    out.freeze(false);
}
