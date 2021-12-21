//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class num_put<charT, OutputIterator>

// iter_type put(iter_type s, ios_base& iob, char_type fill, double v) const;

// TODO(EricWF): This test takes 40+ minutes to build with Clang 3.8 under ASAN or MSAN.
// UNSUPPORTED: asan, msan

#include <locale>
#include <ios>
#include <cassert>
#include <streambuf>
#include <cmath>
#include "test_iterators.h"

typedef std::num_put<char, output_iterator<char*> > F;

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

void test1()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = +0.;
        std::ios ios(0);
        // %g
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0.00000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0;00000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************0.00000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************0;00000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0.00000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************0;00000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+0.00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************0.00000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+0;00000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************0;00000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0.000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0;000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(60);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;00000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void test2()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = 1234567890.125;
        std::ios ios(0);
        // %g
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457e+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457e+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457e+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457e+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457e+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457e+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457e+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1.23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457e+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1;23457e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457E+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457E+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.23457E+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;23457E+09**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457E+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457E+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.23457E+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1.23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;23457E+09*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1;23457E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(60);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1234567890.125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1234567890.125");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******1_234_567_89_0;125");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;12500000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void test3()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = +0.;
        std::ios ios(0);
        fixed(ios);
        // %f
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0************************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0***********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********************+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***********************0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************+0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********************0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0**********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0.0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0.0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0*********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********************+0;0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********************0;0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0.000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0.000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****************+0;000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+****************0;000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;0000000000000000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0.0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;0000000000000000******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******+0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******0;0000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(60);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0.000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0;000000000000000000000000000000000000000000000000000000000000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void test4()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = 1234567890.125;
        std::ios ios(0);
        fixed(ios);
        // %f
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890***************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***************1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***************1234567890");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1234567890.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************+1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**************1234567890");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1234567890.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********+1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890***************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***************1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***************1234567890");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0***********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************1234567890.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890**************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**************+1234567890");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**************1234567890");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0**********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "**********+1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+**********1_234_567_89_0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************+1234567890.");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*************1234567890.");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********+1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*********1_234_567_89_0;");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.1*************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;1*********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.1************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************+1234567890.1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+************1234567890.1");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;1********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********+1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+********1_234_567_89_0;1");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1234567890.125000********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1_234_567_89_0;125000****");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "****1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1234567890.125000*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******1234567890.125000");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1_234_567_89_0;125000***");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "***+1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+***1_234_567_89_0;125000");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {}
            ios.precision(60);
            {}
        }
    }
}

void test5()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = -0.;
        std::ios ios(0);
        scientific(ios);
        // %e
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.e+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0.e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0.e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;e+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0;e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0;e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0e+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.e+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0.e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0.e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;e+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0;e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0;e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.E+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0.E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0.E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;E+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0;E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0;E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0E+00*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************-0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*******************0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.E+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0.E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0.E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;E+00******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0;E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0;E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0e+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0.0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0.0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;0E+00*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0;0E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0;0E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000e+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000e+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0.000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0.000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0;000000E+00************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "************-0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-************0;000000E+00");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {
            }
            ios.precision(60);
            {
            }
        }
    }
}

void test6()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = 1234567890.125;
        std::ios ios(0);
        scientific(ios);
        // %e
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1e+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1e+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1E+09********************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1E+09*******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******************+1E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******************1E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1.E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1.E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************+1;E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+******************1;E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2e+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2e+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2e+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2e+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1.2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1.2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2e+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1;2e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1;2e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.2E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;2E+09******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2E+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2E+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.2E+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1.2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1.2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;2E+09*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************+1;2E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*****************1;2E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
            }
            ios.precision(16);
            {
            }
            ios.precision(60);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000e+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1.234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+1;234567890125000000000000000000000000000000000000000000000000E+09");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void test7()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = -0.;
        std::ios ios(0);
        hexfloat(ios);
        // %a
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0p+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0x0p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0x0p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0.p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0.p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0x0;p+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0x0;p+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0P+0******************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "******************-0X0P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-******************0X0P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0.P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0.P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-0X0;P+0*****************");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*****************-0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "-*****************0X0;P+0");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(16);
            {
            }
            ios.precision(60);
            {
            }
        }
    }
}

void test8()
{
    char str[200];
    output_iterator<char*> iter;
    std::locale lc = std::locale::classic();
    std::locale lg(lc, new my_numpunct);
    const my_facet f(1);
    {
        double v = 1234567890.125;
        std::ios ios(0);
        hexfloat(ios);
        // %a
        {
            ios.precision(0);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(1);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
            ios.precision(6);
            {
            }
            ios.precision(16);
            {
            }
            ios.precision(60);
            {
                nouppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1.26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x1;26580b488p+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0x********1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1.26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1.26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0x1;26580b488p+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0x1;26580b488p+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
                uppercase(ios);
                {
                    noshowpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1.26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X1;26580B488P+30********");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "********0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "0X********1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                    showpos(ios);
                    {
                        noshowpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                        showpoint(ios);
                        {
                            ios.imbue(lc);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1.26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1.26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                            ios.imbue(lg);
                            {
                                ios.width(0);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                left(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+0X1;26580B488P+30*******");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                right(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "*******+0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                                ios.width(25);
                                internal(ios);
                                {
                                    iter = f.put(output_iterator<char*>(str), ios, '*', v);
                                    std::string ex(str, iter.base());
                                    assert(ex == "+*******0X1;26580B488P+30");
                                    assert(ios.width() == 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
}
