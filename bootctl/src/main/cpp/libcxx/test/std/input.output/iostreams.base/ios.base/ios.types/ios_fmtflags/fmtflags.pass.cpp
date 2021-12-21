//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// class ios_base

// static const fmtflags boolalpha;
// static const fmtflags dec;
// static const fmtflags fixed;
// static const fmtflags hex;
// static const fmtflags internal;
// static const fmtflags left;
// static const fmtflags oct;
// static const fmtflags right;
// static const fmtflags scientific;
// static const fmtflags showbase;
// static const fmtflags showpoint;
// static const fmtflags showpos;
// static const fmtflags skipws;
// static const fmtflags unitbuf;
// static const fmtflags uppercase;
// static const fmtflags adjustfield = left | right | internal;
// static const fmtflags basefield   = dec | oct | hex;
// static const fmtflags floatfield  = scientific | fixed;

#include <ios>
#include <cassert>

int main()
{
    assert(std::ios_base::boolalpha);
    assert(std::ios_base::dec);
    assert(std::ios_base::fixed);
    assert(std::ios_base::hex);
    assert(std::ios_base::internal);
    assert(std::ios_base::left);
    assert(std::ios_base::oct);
    assert(std::ios_base::right);
    assert(std::ios_base::scientific);
    assert(std::ios_base::showbase);
    assert(std::ios_base::showpoint);
    assert(std::ios_base::showpos);
    assert(std::ios_base::skipws);
    assert(std::ios_base::unitbuf);
    assert(std::ios_base::uppercase);

    assert
    (
        ( std::ios_base::boolalpha
        & std::ios_base::dec
        & std::ios_base::fixed
        & std::ios_base::hex
        & std::ios_base::internal
        & std::ios_base::left
        & std::ios_base::oct
        & std::ios_base::right
        & std::ios_base::scientific
        & std::ios_base::showbase
        & std::ios_base::showpoint
        & std::ios_base::showpos
        & std::ios_base::skipws
        & std::ios_base::unitbuf
        & std::ios_base::uppercase) == 0
    );

    assert(std::ios_base::adjustfield == (std::ios_base::left
                                        | std::ios_base::right
                                        | std::ios_base::internal));
    assert(std::ios_base::basefield == (std::ios_base::dec
                                      | std::ios_base::oct
                                      | std::ios_base::hex));
    assert(std::ios_base::floatfield == (std::ios_base::scientific
                                       | std::ios_base::fixed));
}
