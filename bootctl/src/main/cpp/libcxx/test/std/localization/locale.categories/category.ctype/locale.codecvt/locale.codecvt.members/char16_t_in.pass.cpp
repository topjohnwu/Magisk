//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<char16_t, char, mbstate_t>

// result in(stateT& state,
//           const externT* from, const externT* from_end, const externT*& from_next,
//           internT* to, internT* to_end, internT*& to_next) const;

#include <locale>
#include <string>
#include <vector>
#include <cassert>

typedef std::codecvt<char16_t, char, std::mbstate_t> F;

int main()
{
    std::locale l = std::locale::classic();
    const char from[] = "some text";
    F::intern_type to[9];
    const F& f = std::use_facet<F>(l);
    std::mbstate_t mbs = {};
    const char* from_next = 0;
    F::intern_type* to_next = 0;
    assert(f.in(mbs, from, from + 9, from_next,
                     to, to + 9, to_next) == F::ok);
    assert(from_next - from == 9);
    assert(to_next - to == 9);
    for (unsigned i = 0; i < 9; ++i)
        assert(to[i] == from[i]);
}
