//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int stoi(const string& str, size_t *idx = 0, int base = 10);
// int stoi(const wstring& str, size_t *idx = 0, int base = 10);

#include <string>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

int main()
{
    assert(std::stoi("0") == 0);
    assert(std::stoi(L"0") == 0);
    assert(std::stoi("-0") == 0);
    assert(std::stoi(L"-0") == 0);
    assert(std::stoi("-10") == -10);
    assert(std::stoi(L"-10") == -10);
    assert(std::stoi(" 10") == 10);
    assert(std::stoi(L" 10") == 10);
    size_t idx = 0;
    assert(std::stoi("10g", &idx, 16) == 16);
    assert(idx == 2);
    idx = 0;
    assert(std::stoi(L"10g", &idx, 16) == 16);
    assert(idx == 2);
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (std::numeric_limits<long>::max() > std::numeric_limits<int>::max())
    {
        try
        {
            std::stoi("0x100000000", &idx, 16);
            assert(false);
        }
        catch (const std::out_of_range&)
        {
        }
        try
        {
            std::stoi(L"0x100000000", &idx, 16);
            assert(false);
        }
        catch (const std::out_of_range&)
        {
        }
    }
    idx = 0;
    try
    {
        std::stoi("", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoi(L"", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoi("  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoi(L"  - 8", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoi("a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
    try
    {
        std::stoi(L"a1", &idx);
        assert(false);
    }
    catch (const std::invalid_argument&)
    {
        assert(idx == 0);
    }
#endif
}
