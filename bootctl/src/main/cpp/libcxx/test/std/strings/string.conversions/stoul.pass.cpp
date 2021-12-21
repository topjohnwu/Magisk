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

// unsigned long stoul(const string& str, size_t *idx = 0, int base = 10);
// unsigned long stoul(const wstring& str, size_t *idx = 0, int base = 10);

#include <string>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

int main()
{
    assert(std::stoul("0") == 0);
    assert(std::stoul(L"0") == 0);
    assert(std::stoul("-0") == 0);
    assert(std::stoul(L"-0") == 0);
    assert(std::stoul(" 10") == 10);
    assert(std::stoul(L" 10") == 10);
    size_t idx = 0;
    assert(std::stoul("10g", &idx, 16) == 16);
    assert(idx == 2);
    idx = 0;
    assert(std::stoul(L"10g", &idx, 16) == 16);
    assert(idx == 2);
#ifndef TEST_HAS_NO_EXCEPTIONS
    idx = 0;
    try
    {
        std::stoul("", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul(L"", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul("  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul(L"  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul("a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul(L"a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
//  LWG issue #2009
    try
    {
        std::stoul("9999999999999999999999999999999999999999999999999", &idx);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoul(L"9999999999999999999999999999999999999999999999999", &idx);
        assert(false);
    }
    catch (const std::out_of_range&)
    {
        assert(idx == 0);
    }
#endif
}
