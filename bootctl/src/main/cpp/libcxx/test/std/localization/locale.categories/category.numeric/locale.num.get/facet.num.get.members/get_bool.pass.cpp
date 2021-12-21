//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class num_get<charT, InputIterator>

// iter_type get(iter_type in, iter_type end, ios_base&,
//               ios_base::iostate& err, bool& v) const;

#include <locale>
#include <ios>
#include <cassert>
#include <streambuf>
#include "test_iterators.h"

typedef std::num_get<char, input_iterator<const char*> > F;

class my_facet
    : public F
{
public:
    explicit my_facet(std::size_t refs = 0)
        : F(refs) {}
};

class p1
    : public std::numpunct<char>
{
public:
    p1() : std::numpunct<char>() {}

protected:
    virtual string_type do_truename() const {return "a";}
    virtual string_type do_falsename() const {return "abb";}
};

class p2
    : public std::numpunct<char>
{
public:
    p2() : std::numpunct<char>() {}

protected:
    virtual string_type do_truename() const {return "a";}
    virtual string_type do_falsename() const {return "ab";}
};

int main()
{
    const my_facet f(1);
    std::ios ios(0);
    {
        const char str[] = "1";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(b == true);
    }
    {
        const char str[] = "0";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(b == false);
    }
    {
        const char str[] = "12";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(b == true);
    }
    {
        const char str[] = "*12";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+0);
        assert(err == ios.failbit);
        assert(b == false);
    }
    boolalpha(ios);
    {
        const char str[] = "1";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+0);
        assert(err == ios.failbit);
        assert(b == false);
    }
    {
        const char str[] = "true";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(b == true);
    }
    {
        const char str[] = "false";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, b);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(b == false);
    }
    ios.imbue(std::locale(ios.getloc(), new p1));
    {
        const char str[] = "a";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+1),
                  ios, err, b);
        assert(iter.base() == str+1);
        assert(err == ios.eofbit);
        assert(b == true);
    }
    {
        const char str[] = "abc";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+3),
                  ios, err, b);
        assert(iter.base() == str+2);
        assert(err == ios.failbit);
        assert(b == false);
    }
    {
        const char str[] = "acc";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+3),
                  ios, err, b);
        assert(iter.base() == str+1);
        assert(err == ios.goodbit);
        assert(b == true);
    }
    ios.imbue(std::locale(ios.getloc(), new p2));
    {
        const char str[] = "a";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+1),
                  ios, err, b);
        assert(iter.base() == str+1);
        assert(err == ios.eofbit);
        assert(b == true);
    }
    {
        const char str[] = "ab";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+2),
                  ios, err, b);
        assert(iter.base() == str+2);
        assert(err == ios.eofbit);
        assert(b == false);
    }
    {
        const char str[] = "abc";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+3),
                  ios, err, b);
        assert(iter.base() == str+2);
        assert(err == ios.goodbit);
        assert(b == false);
    }
    {
        const char str[] = "ac";
        std::ios_base::iostate err = ios.goodbit;
        bool b;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+2),
                  ios, err, b);
        assert(iter.base() == str+1);
        assert(err == ios.goodbit);
        assert(b == true);
    }
}
