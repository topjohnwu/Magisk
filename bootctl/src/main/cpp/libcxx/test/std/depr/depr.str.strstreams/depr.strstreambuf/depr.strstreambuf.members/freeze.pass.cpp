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

// void freeze(bool freezefl = true);

#include <strstream>
#include <cassert>

int main()
{
    {
        std::strstreambuf sb;
        sb.freeze(true);
        assert(sb.sputc('a') == EOF);
        sb.freeze(false);
        assert(sb.sputc('a') == 'a');
    }
}
