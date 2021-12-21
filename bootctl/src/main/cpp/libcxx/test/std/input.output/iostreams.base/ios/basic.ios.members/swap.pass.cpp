//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8

// <ios>

// template <class charT, class traits> class basic_ios

// void move(basic_ios&& rhs);

#include <ios>
#include <streambuf>
#include <cassert>

#include "platform_support.h" // locale name macros

struct testbuf
    : public std::streambuf
{
};

struct testios
    : public std::ios
{
    testios(std::streambuf* p) : std::ios(p) {}
    void swap(std::ios& x) {std::ios::swap(x);}
};

bool f1_called = false;
bool f2_called = false;

bool g1_called = false;
bool g2_called = false;
bool g3_called = false;

void f1(std::ios_base::event, std::ios_base&, int index)
{
    assert(index == 4);
    f1_called = true;
}

void f2(std::ios_base::event, std::ios_base&, int index)
{
    assert(index == 5);
    f2_called = true;
}

void g1(std::ios_base::event, std::ios_base&, int index)
{
    assert(index == 7);
    g1_called = true;
}

void g2(std::ios_base::event, std::ios_base&, int index)
{
    assert(index == 8);
    g2_called = true;
}

void g3(std::ios_base::event, std::ios_base&, int index)
{
    assert(index == 9);
    g3_called = true;
}

int main()
{
    testbuf sb1;
    testios ios1(&sb1);
    ios1.flags(std::ios::boolalpha | std::ios::dec | std::ios::fixed);
    ios1.precision(1);
    ios1.width(11);
    ios1.imbue(std::locale(LOCALE_en_US_UTF_8));
    ios1.exceptions(std::ios::failbit);
    ios1.setstate(std::ios::eofbit);
    ios1.register_callback(f1, 4);
    ios1.register_callback(f2, 5);
    ios1.iword(0) = 1;
    ios1.iword(1) = 2;
    ios1.iword(2) = 3;
    char c1, c2, c3;
    ios1.pword(0) = &c1;
    ios1.pword(1) = &c2;
    ios1.pword(2) = &c3;
    ios1.tie((std::ostream*)1);
    ios1.fill('1');

    testbuf sb2;
    testios ios2(&sb2);
    ios2.flags(std::ios::showpoint | std::ios::uppercase);
    ios2.precision(2);
    ios2.width(12);
    ios2.imbue(std::locale(LOCALE_fr_FR_UTF_8));
    ios2.exceptions(std::ios::eofbit);
    ios2.setstate(std::ios::goodbit);
    ios2.register_callback(g1, 7);
    ios2.register_callback(g2, 8);
    ios2.register_callback(g3, 9);
    ios2.iword(0) = 4;
    ios2.iword(1) = 5;
    ios2.iword(2) = 6;
    ios2.iword(3) = 7;
    ios2.iword(4) = 8;
    ios2.iword(5) = 9;
    char d1, d2;
    ios2.pword(0) = &d1;
    ios2.pword(1) = &d2;
    ios2.tie((std::ostream*)2);
    ios2.fill('2');

    ios1.swap(ios2);

    assert(ios1.rdstate() == std::ios::goodbit);
    assert(ios1.rdbuf() == &sb1);
    assert(ios1.flags() == (std::ios::showpoint | std::ios::uppercase));
    assert(ios1.precision() == 2);
    assert(ios1.width() == 12);
    assert(ios1.getloc().name() == LOCALE_fr_FR_UTF_8);
    assert(ios1.exceptions() == std::ios::eofbit);
    assert(!f1_called);
    assert(!f2_called);
    assert(!g1_called);
    assert(!g2_called);
    assert(!g3_called);
    assert(ios1.iword(0) == 4);
    assert(ios1.iword(1) == 5);
    assert(ios1.iword(2) == 6);
    assert(ios1.iword(3) == 7);
    assert(ios1.iword(4) == 8);
    assert(ios1.iword(5) == 9);
    assert(ios1.pword(0) == &d1);
    assert(ios1.pword(1) == &d2);
    assert(ios1.tie() == (std::ostream*)2);
    assert(ios1.fill() == '2');
    ios1.imbue(std::locale("C"));
    assert(!f1_called);
    assert(!f2_called);
    assert(g1_called);
    assert(g2_called);
    assert(g3_called);

    assert(ios2.rdstate() == std::ios::eofbit);
    assert(ios2.rdbuf() == &sb2);
    assert(ios2.flags() == (std::ios::boolalpha | std::ios::dec | std::ios::fixed));
    assert(ios2.precision() == 1);
    assert(ios2.width() == 11);
    assert(ios2.getloc().name() == LOCALE_en_US_UTF_8);
    assert(ios2.exceptions() == std::ios::failbit);
    assert(ios2.iword(0) == 1);
    assert(ios2.iword(1) == 2);
    assert(ios2.iword(2) == 3);
    assert(ios2.pword(0) == &c1);
    assert(ios2.pword(1) == &c2);
    assert(ios2.pword(2) == &c3);
    assert(ios2.tie() == (std::ostream*)1);
    assert(ios2.fill() == '1');
    ios2.imbue(std::locale("C"));
    assert(f1_called);
    assert(f2_called);
}
