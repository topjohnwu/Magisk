//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// explicit tuple(const T&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

int main()
{
    {
        std::tuple<int, char*, std::string, double&> t(2, nullptr, "text");
    }
}
