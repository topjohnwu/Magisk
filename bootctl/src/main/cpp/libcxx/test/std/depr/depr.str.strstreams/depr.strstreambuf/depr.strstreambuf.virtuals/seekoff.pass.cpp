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

// pos_type seekoff(off_type off, ios_base::seekdir way,
//                  ios_base::openmode which = ios_base::in | ios_base::out);

#include <strstream>
#include <cassert>

int main()
{
    {
        char buf[] = "0123456789";
        std::strstreambuf sb(buf, 0);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in | std::ios_base::out) == -1);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.sgetc() == '3');
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.sgetc() == '6');
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.sgetc() == '7');
    }
    {
        char buf[] = "0123456789";
        std::strstreambuf sb(buf, 0, buf);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::in) == 3);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::in) == 6);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::in) == 7);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out | std::ios_base::in) == 3);
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out | std::ios_base::in) == -1);
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out | std::ios_base::in) == 7);
        assert(sb.pubseekoff(3, std::ios_base::beg, std::ios_base::out) == 3);
        assert(sb.sputc('a') == 'a');
        assert(sb.str() == std::string("012a456789"));
        assert(sb.pubseekoff(3, std::ios_base::cur, std::ios_base::out) == 7);
        assert(sb.sputc('b') == 'b');
        assert(sb.str() == std::string("012a456b89"));
        assert(sb.pubseekoff(-3, std::ios_base::end, std::ios_base::out) == 7);
        assert(sb.sputc('c') == 'c');
        assert(sb.str() == std::string("012a456c89"));
    }
}
