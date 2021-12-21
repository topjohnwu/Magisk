//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// Period shall be a specialization of ratio, diagnostic required.

#include <chrono>

template <int N, int D = 1>
class Ratio
{
public:
    static const int num = N;
    static const int den = D;
};

int main()
{
    typedef std::chrono::duration<int, Ratio<1> > D;
    D d;
}
