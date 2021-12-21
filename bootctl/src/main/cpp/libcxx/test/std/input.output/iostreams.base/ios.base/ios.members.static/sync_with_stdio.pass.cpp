//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// bool sync_with_stdio(bool sync = true);

#include <ios>
#include <cassert>

int main()
{
    assert( std::ios_base::sync_with_stdio(false));
    assert(!std::ios_base::sync_with_stdio(false));
    assert(!std::ios_base::sync_with_stdio(true));
    assert( std::ios_base::sync_with_stdio(true));
    assert( std::ios_base::sync_with_stdio());
    assert( std::ios_base::sync_with_stdio(false));
    assert(!std::ios_base::sync_with_stdio());
    assert( std::ios_base::sync_with_stdio());
}
