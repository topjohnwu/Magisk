//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// default_delete[]

// template <class U>
//   default_delete(const default_delete<U[]>&);
//
// This constructor shall not participate in overload resolution unless
//   U(*)[] is convertible to T(*)[].

#include <memory>
#include <cassert>

int main()
{
    std::default_delete<int[]> d1;
    std::default_delete<const int[]> d2 = d1;
    ((void)d2);
}
