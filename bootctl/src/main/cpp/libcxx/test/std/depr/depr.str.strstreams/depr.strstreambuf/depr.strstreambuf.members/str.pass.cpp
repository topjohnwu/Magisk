//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class strstreambuf

// char* str();

#include <strstream>
#include <cassert>

int main()
{
    {
        std::strstreambuf sb;
        assert(sb.sputc('a') == 'a');
        assert(sb.sputc(0) == 0);
        assert(sb.str() == std::string("a"));
        sb.freeze(false);
    }
}
