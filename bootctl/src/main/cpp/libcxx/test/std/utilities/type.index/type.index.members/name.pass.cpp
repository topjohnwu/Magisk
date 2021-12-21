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

// const char* name() const;

#include <typeindex>
#include <string>
#include <cassert>

int main()
{
    const std::type_info& ti = typeid(int);
    std::type_index t1 = typeid(int);
    assert(std::string(t1.name()) == ti.name());
}
