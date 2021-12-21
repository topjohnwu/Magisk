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

// strstreambuf(const char* gnext_arg, streamsize n);

#include <strstream>
#include <cassert>

int main()
{
    {
        const char buf[] = "abcd";
        std::strstreambuf sb(buf, sizeof(buf));
        assert(sb.sgetc() == 'a');
        assert(sb.snextc() == 'b');
        assert(sb.snextc() == 'c');
        assert(sb.snextc() == 'd');
        assert(sb.snextc() == 0);
        assert(sb.snextc() == EOF);
    }
    {
        const char buf[] = "abcd";
        std::strstreambuf sb(buf, 0);
        assert(sb.sgetc() == 'a');
        assert(sb.snextc() == 'b');
        assert(sb.snextc() == 'c');
        assert(sb.snextc() == 'd');
        assert(sb.snextc() == EOF);
    }
}
