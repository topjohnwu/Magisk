//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test type_info

#include <typeinfo>
#include <cstring>
#include <cassert>

int main()
{
    const std::type_info& t1 = typeid(int);
    const std::type_info& t2 = typeid(int);
    const std::type_info& t3 = typeid(short);
    assert(t1.hash_code() == t2.hash_code());
    assert(t1.hash_code() != t3.hash_code());
}
