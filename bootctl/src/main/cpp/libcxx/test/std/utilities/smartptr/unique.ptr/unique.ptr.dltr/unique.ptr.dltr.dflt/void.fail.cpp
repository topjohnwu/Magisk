//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// default_delete

// Test that default_delete's operator() requires a complete type

#include <memory>
#include <cassert>

int main()
{
    std::default_delete<const void> d;
    const void* p = 0;
    d(p);
}
