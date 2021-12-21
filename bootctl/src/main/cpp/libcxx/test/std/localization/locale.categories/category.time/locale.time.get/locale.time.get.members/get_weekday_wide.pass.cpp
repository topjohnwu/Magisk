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
// get_weekday(iter_type s, iter_type end, ios_base& str,
//             ios_base::iostate& err, tm* t) const;

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
        const wchar_t in[] = L"Sun";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 0);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Suny";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 0);
        assert(err == std::ios_base::goodbit);
    }
    {
        const wchar_t in[] = L"Sund";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+4);
        assert(t.tm_wday == 0);
        assert(err == (std::ios_base::failbit | std::ios_base::eofbit));
    }
    {
        const wchar_t in[] = L"sun";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 0);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"sunday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+6);
        assert(t.tm_wday == 0);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Mon";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Mony";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 1);
        assert(err == std::ios_base::goodbit);
    }
    {
        const wchar_t in[] = L"Mond";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+4);
        assert(t.tm_wday == 0);
        assert(err == (std::ios_base::failbit | std::ios_base::eofbit));
    }
    {
        const wchar_t in[] = L"mon";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"monday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+6);
        assert(t.tm_wday == 1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Tue";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 2);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Tuesday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+7);
        assert(t.tm_wday == 2);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Wed";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 3);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Wednesday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+9);
        assert(t.tm_wday == 3);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Thu";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 4);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Thursday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+8);
        assert(t.tm_wday == 4);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Fri";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 5);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Friday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+6);
        assert(t.tm_wday == 5);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Sat";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+3);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const wchar_t in[] = L"Saturday";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get_weekday(I(in), I(in+sizeof(in)/sizeof(in[0])-1), ios, err, &t);
        assert(i.base() == in+8);
        assert(t.tm_wday == 6);
        assert(err == std::ios_base::eofbit);
    }
}
