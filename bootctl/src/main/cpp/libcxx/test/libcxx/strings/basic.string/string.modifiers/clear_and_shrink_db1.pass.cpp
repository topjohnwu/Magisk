//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// Call __clear_and_shrink() and ensure string invariants hold

#if _LIBCPP_DEBUG >= 1

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <string>
#include <cassert>

int main()
{
    std::string l = "Long string so that allocation definitely, for sure, absolutely happens. Probably.";
    std::string s = "short";

    assert(l.__invariants());
    assert(s.__invariants());

    s.__clear_and_shrink();
    assert(s.__invariants());
    assert(s.size() == 0);

    {
    std::string::size_type cap = l.capacity();
    l.__clear_and_shrink();
    assert(l.__invariants());
    assert(l.size() == 0);
    assert(l.capacity() < cap);
    }
}

#else

int main()
{
}

#endif
