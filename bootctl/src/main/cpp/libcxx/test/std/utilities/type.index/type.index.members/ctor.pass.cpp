//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <typeindex>

// class type_index

// type_index(const type_info& rhs);

#include <typeinfo>
#include <typeindex>
#include <cassert>

int main()
{
    std::type_info const & info = typeid(int);
    std::type_index t1(info);
    assert(t1.name() == info.name());
}
