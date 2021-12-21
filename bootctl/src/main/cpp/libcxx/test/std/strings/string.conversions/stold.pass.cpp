//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// long double stold(const string& str, size_t *idx = 0);
// long double stold(const wstring& str, size_t *idx = 0);

#include <iostream>

#include <string>
#include <cmath>
#include <cassert>

#include "test_macros.h"

int main()
{
    assert(std::stold("0") == 0);
    assert(std::stold(L"0") == 0);
    assert(std::stold("-0") == 0);
    assert(std::stold(L"-0") == 0);
    assert(std::stold("-10") == -10);
    assert(std::stold(L"-10.5") == -10.5);
    assert(std::stold(" 10") == 10);
    assert(std::stold(L" 10") == 10);
    size_t idx = 0;
    assert(std::stold("10g", &idx) == 10);
    assert(idx == 2);
    idx = 0;
    assert(std::stold(L"10g", &idx) == 10);
    assert(idx == 2);
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
#endif
    {
        assert(std::stold("1.e60", &idx) == 1.e60L);
        assert(idx == 5);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (const std::out_of_range&)
    {
        assert(false);
    }
    try
#endif
    {
        assert(std::stold(L"1.e60", &idx) == 1.e60L);
        assert(idx == 5);
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
    {
        assert(std::stold("1.e6000", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
    {
        assert(std::stold(L"1.e6000", &idx) == INFINITY);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
#endif
    {
        assert(std::stold("INF", &idx) == INFINITY);
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
        assert(std::stold(L"INF", &idx) == INFINITY);
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
        assert(std::isnan(std::stold("NAN", &idx)));
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
        assert(std::isnan(std::stold(L"NAN", &idx)));
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
        std::stold("", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stold(L"", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stold("  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stold(L"  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stold("a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stold(L"a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
#endif
}
