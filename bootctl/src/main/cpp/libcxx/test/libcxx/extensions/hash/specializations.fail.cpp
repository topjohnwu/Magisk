//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <ext/hash_map>
#include <string>

int main()
{
    assert(__gnu_cxx::hash<std::string>()(std::string()) == 0);  // error
}
