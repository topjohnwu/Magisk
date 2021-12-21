//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <map>

// class map

// mapped_type& operator[](const key_type& k);

// https://bugs.llvm.org/show_bug.cgi?id=16542

#include <map>


#include <tuple>


int main()
{
    using namespace std;
    map<tuple<int,int>, size_t> m;
    m[make_tuple(2,3)]=7;
}
