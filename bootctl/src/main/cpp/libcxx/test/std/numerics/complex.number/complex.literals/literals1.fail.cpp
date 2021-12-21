// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
#include <complex>
#include <cassert>

#include "test_macros.h"

int main()
{
    std::complex<float> foo  = 1.0if;  // should fail w/conversion operator not found
}
