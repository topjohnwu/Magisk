//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostream_iterator

// ostream_iterator& operator=(const T& value);

#include <iterator>
#include <sstream>
#include <cassert>

#include "test_macros.h"

#if defined(TEST_COMPILER_CLANG)
#pragma clang diagnostic ignored "-Wliteral-conversion"
#endif

#ifdef TEST_COMPILER_C1XX
#pragma warning(disable: 4244) // conversion from 'X' to 'Y', possible loss of data
#endif

int main()
{
    {
        std::ostringstream outf;
        std::ostream_iterator<int> i(outf);
        i = 2.4;
        assert(outf.str() == "2");
    }
    {
        std::ostringstream outf;
        std::ostream_iterator<int> i(outf, ", ");
        i = 2.4;
        assert(outf.str() == "2, ");
    }
    {
        std::wostringstream outf;
        std::ostream_iterator<int, wchar_t> i(outf);
        i = 2.4;
        assert(outf.str() == L"2");
    }
    {
        std::wostringstream outf;
        std::ostream_iterator<int, wchar_t> i(outf, L", ");
        i = 2.4;
        assert(outf.str() == L"2, ");
    }
}
