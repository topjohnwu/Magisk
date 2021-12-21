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

// int_type pbackfail(int_type c = EOF);

#include <strstream>
#include <cassert>

struct test
    : public std::strstreambuf
{
    typedef std::strstreambuf base;
    test(char* gnext_arg, std::streamsize n, char* pbeg_arg = 0)
        : base(gnext_arg, n, pbeg_arg) {}
    test(const char* gnext_arg, std::streamsize n)
        : base(gnext_arg, n) {}

    virtual int_type pbackfail(int_type c = EOF) {return base::pbackfail(c);}
};

int main()
{
    {
        const char buf[] = "123";
        test sb(buf, 0);
        assert(sb.sgetc() == '1');
        assert(sb.snextc() == '2');
        assert(sb.snextc() == '3');
        assert(sb.sgetc() == '3');
        assert(sb.snextc() == EOF);
        assert(sb.pbackfail('3') == '3');
        assert(sb.pbackfail('3') == EOF);
        assert(sb.pbackfail('2') == '2');
        assert(sb.pbackfail(EOF) != EOF);
        assert(sb.pbackfail(EOF) == EOF);
        assert(sb.str() == std::string("123"));
    }
    {
        char buf[] = "123";
        test sb(buf, 0);
        assert(sb.sgetc() == '1');
        assert(sb.snextc() == '2');
        assert(sb.snextc() == '3');
        assert(sb.sgetc() == '3');
        assert(sb.snextc() == EOF);
        assert(sb.pbackfail('3') == '3');
        assert(sb.pbackfail('3') == '3');
        assert(sb.pbackfail(EOF) != EOF);
        assert(sb.pbackfail(EOF) == EOF);
        assert(sb.str() == std::string("133"));
    }
}
