//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class ostrstream

// int pcount() const;

#include <strstream>
#include <cassert>

int main()
{
    {
        std::ostrstream out;
        assert(out.pcount() == 0);
        out << 123 << ' ' << 4.5 << ' ' << "dog";
        assert(out.pcount() == 11);
    }
}
