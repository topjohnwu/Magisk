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

// strstreambuf(signed char* gnext_arg, streamsize n, signed char* pbeg_arg = 0);

#include <strstream>
#include <cassert>
#include <cstring>

int main()
{
    {
        signed char buf[] = "abcd";
        std::strstreambuf sb(buf, sizeof(buf));
        assert(sb.sgetc() == 'a');
        assert(sb.snextc() == 'b');
        assert(sb.snextc() == 'c');
        assert(sb.snextc() == 'd');
        assert(sb.snextc() == 0);
        assert(sb.snextc() == EOF);
    }
    {
        signed char buf[] = "abcd";
        std::strstreambuf sb(buf, 0);
        assert(sb.sgetc() == 'a');
        assert(sb.snextc() == 'b');
        assert(sb.snextc() == 'c');
        assert(sb.snextc() == 'd');
        assert(sb.snextc() == EOF);
    }
    {
        signed char buf[] = "abcd";
        std::strstreambuf sb(buf, sizeof(buf), buf);
        assert(sb.sgetc() == EOF);
        assert(sb.sputc('e') == 'e');
        assert(sb.sputc('f') == 'f');
        assert(sb.sputc('g') == 'g');
        assert(sb.sputc('h') == 'h');
        assert(sb.sputc('i') == 'i');
        assert(sb.sputc('j') == EOF);
        assert(sb.sgetc() == 'e');
        assert(sb.snextc() == 'f');
        assert(sb.snextc() == 'g');
        assert(sb.snextc() == 'h');
        assert(sb.snextc() == 'i');
        assert(sb.snextc() == EOF);
    }
    {
        signed char buf[] = "abcd";
        std::strstreambuf sb(buf, 0, buf);
        assert(sb.sgetc() == EOF);
        assert(sb.sputc('e') == 'e');
        assert(sb.sputc('f') == 'f');
        assert(sb.sputc('g') == 'g');
        assert(sb.sputc('h') == 'h');
        assert(sb.sputc('i') == EOF);
        assert(sb.sgetc() == 'e');
        assert(sb.snextc() == 'f');
        assert(sb.snextc() == 'g');
        assert(sb.snextc() == 'h');
        assert(sb.snextc() == EOF);
    }
    {
        signed char buf[10] = "abcd";
        std::size_t s = std::strlen((char*)buf);
        std::strstreambuf sb(buf, sizeof(buf) - s, buf + s);
        assert(sb.sgetc() == 'a');
        assert(sb.snextc() == 'b');
        assert(sb.snextc() == 'c');
        assert(sb.snextc() == 'd');
        assert(sb.snextc() == EOF);
        assert(sb.sputc('e') == 'e');
        assert(sb.sputc('f') == 'f');
        assert(sb.sputc('g') == 'g');
        assert(sb.sputc('h') == 'h');
        assert(sb.sputc('i') == 'i');
        assert(sb.sputc('j') == 'j');
        assert(sb.sputc('j') == EOF);
        assert(sb.sgetc() == 'e');
        assert(sb.snextc() == 'f');
        assert(sb.snextc() == 'g');
        assert(sb.snextc() == 'h');
        assert(sb.snextc() == 'i');
        assert(sb.snextc() == 'j');
        assert(sb.snextc() == EOF);
    }
}
