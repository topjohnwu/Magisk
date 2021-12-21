//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ctgmath>

#include <ctgmath>

int main()
{
    std::complex<double> cd;
    (void)cd;
    double x = std::sin(0);
    ((void)x); // Prevent unused warning
}
