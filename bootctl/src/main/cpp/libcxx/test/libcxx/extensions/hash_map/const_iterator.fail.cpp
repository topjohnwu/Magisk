//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <ext/hash_map>

int main()
{
    __gnu_cxx::hash_map<int, int> m;
    m[1] = 1;
    const __gnu_cxx::hash_map<int, int> &cm = m;
    cm.find(1)->second = 2;  // error
}
