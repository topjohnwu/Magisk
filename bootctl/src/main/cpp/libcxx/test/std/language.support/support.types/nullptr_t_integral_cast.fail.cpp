//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// typedef decltype(nullptr) nullptr_t;

#include <cstddef>

int main()
{
    std::ptrdiff_t i = static_cast<std::ptrdiff_t>(nullptr);
}
