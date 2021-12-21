//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <ios>

// class ios_base

// void register_callback(event_callback fn, int index);

#include <ios>
#include <string>
#include <locale>
#include <cassert>

#include "platform_support.h" // locale name macros

class test
    : public std::ios
{
public:
    test()
    {
        init(0);
    }
};

int f1_called = 0;

void f1(std::ios_base::event ev, std::ios_base& stream, int index)
{
    if (ev == std::ios_base::imbue_event)
    {
        assert(stream.getloc().name() == LOCALE_en_US_UTF_8);
        assert(index == 4);
        ++f1_called;
    }
}

int main()
{
    test t;
    std::ios_base& b = t;
    b.register_callback(f1, 4);
    b.register_callback(f1, 4);
    b.register_callback(f1, 4);
    std::locale l = b.imbue(std::locale(LOCALE_en_US_UTF_8));
    assert(f1_called == 3);
}
