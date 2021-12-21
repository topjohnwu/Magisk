//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// iterator insert(const_iterator p, charT c);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <string>
#include <stdexcept>
#include <cassert>


int main()
{
#if _LIBCPP_DEBUG >= 1
    {
        typedef std::string S;
        S s;
        S s2;
        s.insert(s2.begin(), '1');
        assert(false);
    }
#endif
}
