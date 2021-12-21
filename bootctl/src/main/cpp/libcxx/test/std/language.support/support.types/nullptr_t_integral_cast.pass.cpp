//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// NOTE: nullptr_t emulation cannot handle a reinterpret_cast to an
// integral type
// XFAIL: c++98, c++03

// typedef decltype(nullptr) nullptr_t;


#include <cstddef>
#include <cassert>

int main()
{
    std::ptrdiff_t i = reinterpret_cast<std::ptrdiff_t>(nullptr);
    assert(i == 0);
}
