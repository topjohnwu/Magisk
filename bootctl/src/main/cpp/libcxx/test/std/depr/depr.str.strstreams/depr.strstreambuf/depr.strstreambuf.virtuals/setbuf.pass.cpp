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

// streambuf* setbuf(char* s, streamsize n);

#include <strstream>
#include <cassert>

int main()
{
    {
        char buf[] = "0123456789";
        std::strstreambuf sb(buf, 0);
        assert(sb.pubsetbuf(0, 0) == &sb);
        assert(sb.str() == std::string("0123456789"));
    }
}
