//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <streambuf>

// template <class charT, class traits = char_traits<charT> >
// class basic_streambuf;

// int_type sungetc();

#include <streambuf>
#include <cassert>

int pbackfail_called = 0;

struct test
    : public std::basic_streambuf<char>
{
    typedef std::basic_streambuf<char> base;

    test() {}

    void setg(char* gbeg, char* gnext, char* gend)
    {
        base::setg(gbeg, gnext, gend);
    }

protected:
    int_type pbackfail(int_type = traits_type::eof())
    {
        ++pbackfail_called;
        return 'a';
    }
};

int main()
{
    {
        test t;
        assert(pbackfail_called == 0);
        assert(t.sungetc() == 'a');
        assert(pbackfail_called == 1);
        char in[] = "ABC";
        t.setg(in, in+1, in+sizeof(in));
        assert(t.sungetc() == 'A');
        assert(pbackfail_called == 1);
        assert(t.sungetc() == 'a');
        assert(pbackfail_called == 2);
    }
}
