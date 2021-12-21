//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// PR11871
// XFAIL: with_system_cxx_lib=macosx10.7

// <locale>

// class num_get<charT, InputIterator>

// iter_type get(iter_type in, iter_type end, ios_base&,
//               ios_base::iostate& err, float& v) const;

#include <locale>
#include <ios>
#include <cassert>
#include <streambuf>
#include <cmath>
#include "test_iterators.h"
#include "hexfloat.h"

typedef std::num_get<char, input_iterator<const char*> > F;

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
    float v = -1;
    {
        const char str[] = "123";
        assert((ios.flags() & ios.basefield) == ios.dec);
        assert(ios.getloc().name() == "C");
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == 123);
    }
    {
        const char str[] = "-123";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == -123);
    }
    {
        const char str[] = "123.5";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == 123.5);
    }
    {
        const char str[] = "125e-1";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == 125e-1);
    }
    {
        const char str[] = "0x125p-1";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == hexfloat<float>(0x125, 0, -1));
    }
    {
        const char str[] = "inf";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "INF";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "-inf";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == -INFINITY);
    }
    {
        const char str[] = "-INF";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == -INFINITY);
    }
    {
        const char str[] = "nan";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(std::isnan(v));
    }
    {
        const char str[] = "NAN";
        hex(ios);
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(std::isnan(v));
    }
    {
        v = -1;
        const char str[] = "3.40283e+39"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == HUGE_VALF);
    }
    {
        v = -1;
        const char str[] = "-3.40283e+38"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == -HUGE_VALF);

    }
    {
        v = -1;
        const char str[] = "2-";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+1);
        assert(err == ios.goodbit);
        assert(v == 2);
    }
}
