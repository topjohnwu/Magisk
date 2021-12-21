//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// PR14919 was fixed in r172447, out_of_range wasn't thrown before.
// XFAIL: with_system_cxx_lib=macosx10.7
// XFAIL: with_system_cxx_lib=macosx10.8

// <string>

// float stof(const string& str, size_t *idx = 0);
// float stof(const wstring& str, size_t *idx = 0);

#include <string>
#include <cmath>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

int main()
{
    assert(std::stof("0") == 0);
    assert(std::stof(L"0") == 0);
    assert(std::stof("-0") == 0);
    assert(std::stof(L"-0") == 0);
    assert(std::stof("-10") == -10);
    assert(std::stof(L"-10.5") == -10.5);
    assert(std::stof(" 10") == 10);
    assert(std::stof(L" 10") == 10);
    size_t idx = 0;
    assert(std::stof("10g", &idx) == 10);
    assert(idx == 2);
    idx = 0;
    assert(std::stof(L"10g", &idx) == 10);
    assert(idx == 2);
#ifndef TEST_HAS_NO_EXCEPTIONS
    idx = 0;
    try
    {
        assert(std::stof("1.e60", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
    {
        assert(std::stof(L"1.e60", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    idx = 0;
    try
    {
        assert(std::stof("1.e360", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
    {
        assert(std::stof(L"1.e360", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
#endif
    {
        assert(std::stof("INF", &idx) == INFINITY);
        assert(idx == 3);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (const std::out_of_range&)
    {
        assert(false);
    }
#endif
    idx = 0;
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
#endif
    {
        assert(std::stof(L"INF", &idx) == INFINITY);
        assert(idx == 3);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (const std::out_of_range&)
    {
        assert(false);
    }
#endif
    idx = 0;
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
#endif
    {
        assert(std::isnan(std::stof("NAN", &idx)));
        assert(idx == 3);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (const std::out_of_range&)
    {
        assert(false);
    }
#endif
    idx = 0;
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
#endif
    {
        assert(std::isnan(std::stof(L"NAN", &idx)));
        assert(idx == 3);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (const std::out_of_range&)
    {
        assert(false);
    }
    idx = 0;
    try
    {
        std::stof("", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stof(L"", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stof("  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stof(L"  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stof("a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stof(L"a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
#endif
}
