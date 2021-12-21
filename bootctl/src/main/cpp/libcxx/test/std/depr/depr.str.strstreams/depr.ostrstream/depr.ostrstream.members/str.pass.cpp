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

// char* str();

#include <strstream>
#include <cassert>

int main()
{
    {
        std::ostrstream out;
        out << 123 << ' ' << 4.5 << ' ' << "dog" << std::ends;
        assert(out.str() == std::string("123 4.5 dog"));
        out.freeze(false);
    }
}
