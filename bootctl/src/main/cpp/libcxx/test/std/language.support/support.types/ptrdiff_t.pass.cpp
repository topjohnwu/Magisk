//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <type_traits>

// ptrdiff_t should:

//  1. be in namespace std.
//  2. be the same sizeof as void*.
//  3. be a signed integral.

int main()
{
    static_assert(sizeof(std::ptrdiff_t) == sizeof(void*),
                  "sizeof(std::ptrdiff_t) == sizeof(void*)");
    static_assert(std::is_signed<std::ptrdiff_t>::value,
                  "std::is_signed<std::ptrdiff_t>::value");
    static_assert(std::is_integral<std::ptrdiff_t>::value,
                  "std::is_integral<std::ptrdiff_t>::value");
}
