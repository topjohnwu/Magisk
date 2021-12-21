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
// PR15445
// XFAIL: with_system_cxx_lib=macosx10.8

// <locale>

// class num_get<charT, InputIterator>

// iter_type get(iter_type in, iter_type end, ios_base&,
//               ios_base::iostate& err, double& v) const;

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

class my_numpunct
    : public std::numpunct<char>
{
public:
    my_numpunct() : std::numpunct<char>() {}

protected:
    virtual char_type do_decimal_point() const {return ';';}
    virtual char_type do_thousands_sep() const {return '_';}
    virtual std::string do_grouping() const {return std::string("\1\2\3");}
};

int main()
{
    const my_facet f(1);
    std::ios ios(0);
    double v = -1;
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
        assert(v == hexfloat<double>(0x125, 0, -1));
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
        const char str[] = "123_456_78_9;125";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+3);
        assert(err == ios.goodbit);
        assert(v == 123);
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
    {
        v = -1;
        const char str[] = "1.79779e+309"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == HUGE_VAL);
    }
    {
        v = -1;
        const char str[] = "-1.79779e+308"; // unrepresentable
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == -HUGE_VAL);
    }
    ios.imbue(std::locale(std::locale(), new my_numpunct));
    {
        v = -1;
        const char str[] = "123_456_78_9;125";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == 123456789.125);
    }
    {
        v = -1;
        const char str[] = "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_"
                           "1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_1_2_3_4_5_6_7_8_9_0_";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
    }
    {
        v = -1;
        const char str[] = "3;14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651e+10";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(std::abs(v - 3.14159265358979e+10)/3.14159265358979e+10 < 1.e-8);
    }
}
