// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11

#include <chrono>
#include <cassert>

int main()
{
    using namespace std::chrono;

    hours h = 4h;
    assert ( h == hours(4));
    auto h2 = 4.0h;
    assert ( h == h2 );

    minutes min = 36min;
    assert ( min == minutes(36));
    auto min2 = 36.0min;
    assert ( min == min2 );

    seconds s = 24s;
    assert ( s == seconds(24));
    auto s2 = 24.0s;
    assert ( s == s2 );

    milliseconds ms = 247ms;
    assert ( ms == milliseconds(247));
    auto ms2 = 247.0ms;
    assert ( ms == ms2 );

    microseconds us = 867us;
    assert ( us == microseconds(867));
    auto us2 = 867.0us;
    assert ( us == us2 );

    nanoseconds ns = 645ns;
    assert ( ns == nanoseconds(645));
    auto ns2 = 645.ns;
    assert ( ns == ns2 );

#if TEST_STD_VER > 17
    assert(Sunday    == weekday(0));
    assert(Monday    == weekday(1));
    assert(Tuesday   == weekday(2));
    assert(Wednesday == weekday(3));
    assert(Thursday  == weekday(4));
    assert(Friday    == weekday(5));
    assert(Saturday  == weekday(6));

    assert(January   == month(1));
    assert(February  == month(2));
    assert(March     == month(3));
    assert(April     == month(4));
    assert(May       == month(5));
    assert(June      == month(6));
    assert(July      == month(7));
    assert(August    == month(8));
    assert(September == month(9));
    assert(October   == month(10));
    assert(November  == month(11));
    assert(December  == month(12));
#endif
}
