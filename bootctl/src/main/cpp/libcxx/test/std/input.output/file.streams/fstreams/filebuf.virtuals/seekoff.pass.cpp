//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// pos_type seekoff(off_type off, ios_base::seekdir way,
//                  ios_base::openmode which = ios_base::in | ios_base::out);
// pos_type seekpos(pos_type sp,
//                  ios_base::openmode which = ios_base::in | ios_base::out);

#include <fstream>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        char buf[10];
        typedef std::filebuf::pos_type pos_type;
        std::filebuf f;
        f.pubsetbuf(buf, sizeof(buf));
        assert(f.open("seekoff.dat", std::ios_base::in | std::ios_base::out
                                                       | std::ios_base::trunc) != 0);
        assert(f.is_open());
        f.sputn("abcdefghijklmnopqrstuvwxyz", 26);
        LIBCPP_ASSERT(buf[0] == 'v');
        pos_type p = f.pubseekoff(-15, std::ios_base::cur);
        assert(p == 11);
        assert(f.sgetc() == 'l');
        f.pubseekoff(0, std::ios_base::beg);
        assert(f.sgetc() == 'a');
        f.pubseekoff(-1, std::ios_base::end);
        assert(f.sgetc() == 'z');
        assert(f.pubseekpos(p) == p);
        assert(f.sgetc() == 'l');
    }
    std::remove("seekoff.dat");
    {
        wchar_t buf[10];
        typedef std::filebuf::pos_type pos_type;
        std::wfilebuf f;
        f.pubsetbuf(buf, sizeof(buf)/sizeof(buf[0]));
        assert(f.open("seekoff.dat", std::ios_base::in | std::ios_base::out
                                                       | std::ios_base::trunc) != 0);
        assert(f.is_open());
        f.sputn(L"abcdefghijklmnopqrstuvwxyz", 26);
        LIBCPP_ASSERT(buf[0] == L'v');
        pos_type p = f.pubseekoff(-15, std::ios_base::cur);
        assert(p == 11);
        assert(f.sgetc() == L'l');
        f.pubseekoff(0, std::ios_base::beg);
        assert(f.sgetc() == L'a');
        f.pubseekoff(-1, std::ios_base::end);
        assert(f.sgetc() == L'z');
        assert(f.pubseekpos(p) == p);
        assert(f.sgetc() == L'l');
    }
    std::remove("seekoff.dat");
}
