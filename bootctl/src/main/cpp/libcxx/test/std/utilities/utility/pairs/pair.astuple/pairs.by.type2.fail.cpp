//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
#include <utility>
#include <complex>

#include <cassert>

int main()
{
    typedef std::complex<float> cf;
    auto t1 = std::make_pair<int, int> ( 42, 43 );
    assert ( std::get<int>(t1) == 42 ); // two ints
}
