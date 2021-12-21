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

// int_type underflow();

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

    base::int_type underflow() {return base::underflow();}
};

int main()
{
    {
        char buf[10] = "123";
        test sb(buf, 0, buf + 3);
        assert(sb.underflow() == '1');
        assert(sb.underflow() == '1');
        assert(sb.snextc() == '2');
        assert(sb.underflow() == '2');
        assert(sb.underflow() == '2');
        assert(sb.snextc() == '3');
        assert(sb.underflow() == '3');
        assert(sb.underflow() == '3');
        assert(sb.snextc() == EOF);
        assert(sb.underflow() == EOF);
        assert(sb.underflow() == EOF);
        sb.sputc('4');
        assert(sb.underflow() == '4');
        assert(sb.underflow() == '4');
    }
}
