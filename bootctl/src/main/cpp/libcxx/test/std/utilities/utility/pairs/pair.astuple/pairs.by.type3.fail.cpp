//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
#include <utility>
#include <complex>

#include <cassert>

int main()
{
    typedef std::unique_ptr<int> upint;
    std::pair<upint, int> t(upint(new int(4)), 23);
    upint p = std::get<upint>(t);
}
