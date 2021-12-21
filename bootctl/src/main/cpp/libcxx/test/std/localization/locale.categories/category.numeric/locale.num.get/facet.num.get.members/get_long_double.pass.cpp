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
//               ios_base::iostate& err, long double& v) const;

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
    long double v = -1;
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
        assert(v == hexfloat<long double>(0x125, 0, -1));
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
        const char str[] = "1.189731495357231765021264e+49321";
        std::ios_base::iostate err = ios.goodbit;
        v = 0;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "1.189731495357231765021264e+49329";
        std::ios_base::iostate err = ios.goodbit;
        v = 0;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "11.189731495357231765021264e+4932";
        std::ios_base::iostate err = ios.goodbit;
        v = 0;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "91.189731495357231765021264e+4932";
        std::ios_base::iostate err = ios.goodbit;
        v = 0;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == INFINITY);
    }
    {
        const char str[] = "304888344611713860501504000000";
        std::ios_base::iostate err = ios.goodbit;
        v = 0;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err != ios.failbit);
        assert(v == 304888344611713860501504000000.0L);
    }
    {
        v = -1;
        const char str[] = "1.19973e+4933"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == HUGE_VALL);
    }
    {
        v = -1;
        const char str[] = "-1.18974e+4932"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == -HUGE_VALL);
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
