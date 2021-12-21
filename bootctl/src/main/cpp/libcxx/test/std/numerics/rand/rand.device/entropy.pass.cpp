//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class random_device;

// double entropy() const;

#include <random>
#include <cassert>

int main()
{
    std::random_device r;
    double e = r.entropy();
    ((void)e); // Prevent unused warning
}
