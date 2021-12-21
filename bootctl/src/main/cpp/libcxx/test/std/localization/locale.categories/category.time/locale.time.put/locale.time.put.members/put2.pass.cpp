//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class time_put<charT, OutputIterator>

// iter_type put(iter_type s, ios_base& str, char_type fill, const tm* t,
//               char format, char modifier = 0) const;

#include <locale>
#include <cassert>
#include "test_iterators.h"

typedef std::time_put<char, output_iterator<char*> > F;

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
    char str[200];
    output_iterator<char*> iter;
    tm t = {};
    t.tm_sec = 6;
    t.tm_min = 3;
    t.tm_hour = 13;
    t.tm_mday = 2;
    t.tm_mon = 4;
    t.tm_year = 109;
    t.tm_wday = 6;
    t.tm_yday = 121;
    t.tm_isdst = 1;
    std::ios ios(0);
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'A');
        std::string ex(str, iter.base());
        assert(ex == "Saturday");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'a');
        std::string ex(str, iter.base());
        assert(ex == "Sat");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'B');
        std::string ex(str, iter.base());
        assert(ex == "May");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'b');
        std::string ex(str, iter.base());
        assert(ex == "May");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'C');
        std::string ex(str, iter.base());
        assert(ex == "20");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'c');
        std::string ex(str, iter.base());
        assert(ex == "Sat May  2 13:03:06 2009");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'D');
        std::string ex(str, iter.base());
        assert(ex == "05/02/09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'd');
        std::string ex(str, iter.base());
        assert(ex == "02");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'c', 'E');
        std::string ex(str, iter.base());
        assert(ex == "Sat May  2 13:03:06 2009");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'C', 'E');
        std::string ex(str, iter.base());
        assert(ex == "20");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'x', 'E');
        std::string ex(str, iter.base());
        assert(ex == "05/02/09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'X', 'E');
        std::string ex(str, iter.base());
        assert(ex == "13:03:06");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'y', 'E');
        std::string ex(str, iter.base());
        assert(ex == "09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'Y', 'E');
        std::string ex(str, iter.base());
        assert(ex == "2009");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'd', 'O');
        std::string ex(str, iter.base());
        assert(ex == "02");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'e', 'O');
        std::string ex(str, iter.base());
        assert(ex == " 2");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'H', 'O');
        std::string ex(str, iter.base());
        assert(ex == "13");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'I', 'O');
        std::string ex(str, iter.base());
        assert(ex == "01");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'm', 'O');
        std::string ex(str, iter.base());
        assert(ex == "05");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'M', 'O');
        std::string ex(str, iter.base());
        assert(ex == "03");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'S', 'O');
        std::string ex(str, iter.base());
        assert(ex == "06");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'u', 'O');
        std::string ex(str, iter.base());
        assert(ex == "6");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'U', 'O');
        std::string ex(str, iter.base());
        assert(ex == "17");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'V', 'O');
        std::string ex(str, iter.base());
        assert(ex == "18");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'w', 'O');
        std::string ex(str, iter.base());
        assert(ex == "6");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'W', 'O');
        std::string ex(str, iter.base());
        assert(ex == "17");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'y', 'O');
        std::string ex(str, iter.base());
        assert(ex == "09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'e');
        std::string ex(str, iter.base());
        assert(ex == " 2");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'F');
        std::string ex(str, iter.base());
        assert(ex == "2009-05-02");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'G');
        std::string ex(str, iter.base());
        assert(ex == "2009");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'g');
        std::string ex(str, iter.base());
        assert(ex == "09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'H');
        std::string ex(str, iter.base());
        assert(ex == "13");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'h');
        std::string ex(str, iter.base());
        assert(ex == "May");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'I');
        std::string ex(str, iter.base());
        assert(ex == "01");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'j');
        std::string ex(str, iter.base());
        assert(ex == "122");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'M');
        std::string ex(str, iter.base());
        assert(ex == "03");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'm');
        std::string ex(str, iter.base());
        assert(ex == "05");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'n');
        std::string ex(str, iter.base());
        assert(ex == "\n");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'p');
        std::string ex(str, iter.base());
        assert(ex == "PM");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'R');
        std::string ex(str, iter.base());
        assert(ex == "13:03");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'r');
        std::string ex(str, iter.base());
        assert(ex == "01:03:06 PM");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'S');
        std::string ex(str, iter.base());
        assert(ex == "06");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'T');
        std::string ex(str, iter.base());
        assert(ex == "13:03:06");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 't');
        std::string ex(str, iter.base());
        assert(ex == "\t");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'U');
        std::string ex(str, iter.base());
        assert(ex == "17");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'u');
        std::string ex(str, iter.base());
        assert(ex == "6");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'V');
        std::string ex(str, iter.base());
        assert(ex == "18");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'W');
        std::string ex(str, iter.base());
        assert(ex == "17");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'w');
        std::string ex(str, iter.base());
        assert(ex == "6");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'X');
        std::string ex(str, iter.base());
        assert(ex == "13:03:06");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'x');
        std::string ex(str, iter.base());
        assert(ex == "05/02/09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'Y');
        std::string ex(str, iter.base());
        assert(ex == "2009");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'y');
        std::string ex(str, iter.base());
        assert(ex == "09");
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'Z');
        std::string ex(str, iter.base());
//        assert(ex == "EDT");  depends on time zone
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, 'z');
        std::string ex(str, iter.base());
//        assert(ex == "-0400");  depends on time zone
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, '+');
        std::string ex(str, iter.base());
//        assert(ex == "Sat May  2 13:03:06 EDT 2009");  depends on time zone
    }
    {
        iter = f.put(output_iterator<char*>(str), ios, '*', &t, '%');
        std::string ex(str, iter.base());
        assert(ex == "%");
    }
}
