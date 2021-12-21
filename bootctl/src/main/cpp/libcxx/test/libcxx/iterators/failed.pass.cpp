//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostreambuf_iterator

// bool failed() const throw();
//
//	Extension: constructing from NULL is UB; we just make it a failed iterator

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::ostreambuf_iterator<char> i(nullptr);
        assert(i.failed());
    }
    {
        std::ostreambuf_iterator<wchar_t> i(nullptr);
        assert(i.failed());
    }
}
