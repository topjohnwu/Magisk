//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class time_get<charT, InputIterator>

// iter_type
// get_time(iter_type s, iter_type end, ios_base& str,
//          ios_base::iostate& err, tm* t) const;

#include <locale>
#include <cassert>
#include "test_iterators.h"

typedef input_iterator<const wchar_t*> I;

typedef std::time_get<wchar_t, I> F;

class my_facet
    : public F
{
public:
    explicit my_facet(std::size_t refs = 0)
        : F(refs) {}
};

int main()
{
    const my_facet f(1);
    std::ios ios(0);
    std::ios_base::iostate err;
    std::tm t;
    {
        const wchar_t in[] = L"0:0:0";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_hour == 0);
        assert(t.tm_min == 0);
        assert(t.tm_sec == 0);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"23:59:60";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+sizeof(in)/sizeof(in[0])-1);
        assert(t.tm_hour == 23);
        assert(t.tm_min == 59);
        assert(t.tm_sec == 60);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"24:59:60";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+2);
        assert(t.tm_hour == 0);
        assert(t.tm_min == 0);
        assert(t.tm_sec == 0);
        assert(err == std::ios_base::failbit);
    }
    {
        const wchar_t in[] = L"23:60:60";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+5);
//         assert(t.tm_hour == 0);
//         assert(t.tm_min == 0);
//         assert(t.tm_sec == 0);
        assert(err == std::ios_base::failbit);
    }
    {
        const wchar_t in[] = L"23:59:61";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+8);
//         assert(t.tm_hour == 0);
//         assert(t.tm_min == 0);
//         assert(t.tm_sec == 0);
        assert(err == (std::ios_base::failbit | std::ios_base::eofbit));
    }
    {
        const wchar_t in[] = L"2:43:221";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+7);
        assert(t.tm_hour == 2);
        assert(t.tm_min == 43);
        assert(t.tm_sec == 22);
        assert(err == std::ios_base::goodbit);
    }
    {
        const wchar_t in[] = L"2.43:221";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_time(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+1);
//         assert(t.tm_hour == 0);
//         assert(t.tm_min == 0);
//         assert(t.tm_sec == 0);
        assert(err == std::ios_base::failbit);
    }
}
