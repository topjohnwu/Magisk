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

// iter_type get(iter_type s, iter_type end, ios_base& f,
//               ios_base::iostate& err, tm *t, char format, char modifier = 0) const;

#include <locale>
#include <cassert>
#include "test_iterators.h"

typedef input_iterator<const char*> I;

typedef std::time_get<char, I> F;

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
        const char in[] = "mon";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'a');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_wday == 1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "wednesdaY";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'A');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_wday == 3);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "June";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'b');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 5);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "Jul";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'B');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 6);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "Thu Jun  6 09:49:10 2009";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'c');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_wday == 4);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 6);
        assert(t.tm_hour == 9);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "11";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'd');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mday == 11);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "2/1/1";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'D');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 1);
        assert(t.tm_mday == 1);
        assert(t.tm_year == 101);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "11";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'e');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mday == 11);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "June";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'h');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 5);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "19";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'H');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 19);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "12";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'm');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 11);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "59";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'M');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_min == 59);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "\t\n ";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'n');
        assert(i.base() == in+sizeof(in)-1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "09:49:10 PM";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'r');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 21);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "09:49:10 AM";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'r');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 9);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "12:49:10 AM";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'r');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 0);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "12:49:10 PM";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'r');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 12);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "09:49";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'R');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 9);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 0);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "60";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'S');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 0);
        assert(t.tm_min == 0);
        assert(t.tm_sec == 60);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "\t\n ";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 't');
        assert(i.base() == in+sizeof(in)-1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "21:49:10";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'T');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 21);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "3";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'w');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_wday == 3);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "06/06/09";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'x');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_mon == 5);
        assert(t.tm_mday == 6);
        assert(t.tm_year == 109);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "21:49:10";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'X');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_hour == 21);
        assert(t.tm_min == 49);
        assert(t.tm_sec == 10);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "68";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'y');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_year == 168);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "68";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, 'Y');
        assert(i.base() == in+sizeof(in)-1);
        assert(t.tm_year == -1832);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char in[] = "%";
        err = std::ios_base::goodbit;
        t = std::tm();
        I i = f.get(I(in), I(in+sizeof(in)-1), ios, err, &t, '%');
        assert(i.base() == in+sizeof(in)-1);
        assert(err == std::ios_base::eofbit);
    }
}
