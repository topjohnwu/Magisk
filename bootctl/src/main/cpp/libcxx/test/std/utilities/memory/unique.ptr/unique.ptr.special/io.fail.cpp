//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
//  Because we don't have a functioning decltype in C++03

// <memory>

// unique_ptr

// template<class CharT, class Traits, class Y, class D>
//   basic_ostream<CharT, Traits>&
//   operator<<(basic_ostream<CharT, Traits>& os, const unique_ptr<Y, D>& p);

//   -?- Remarks: This function shall not participate in overload resolution unless os << p.get() is a valid expression.

#include <memory>
#include <sstream>
#include <cassert>

#include "min_allocator.h"
#include "deleter_types.h"

int main()
{
    std::unique_ptr<int, PointerDeleter<int>> p;
    std::ostringstream os;
    os << p; // expected-error {{invalid operands to binary expression}}
}
