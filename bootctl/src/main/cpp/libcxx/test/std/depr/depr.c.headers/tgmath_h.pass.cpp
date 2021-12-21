//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tgmath.h>

#include <tgmath.h>

int main()
{
    std::complex<double> cd;
    (void)cd;
    double x = sin(1.0);
    (void)x; // to placate scan-build
}
