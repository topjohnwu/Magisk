//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<wchar_t, char, mbstate_t>

// result in(stateT& state,
//           const externT* from, const externT* from_end, const externT*& from_next,
//           internT* to, internT* to_end, internT*& to_next) const;

#include <locale>
#include <string>
#include <vector>
#include <cassert>
#include <cstddef>

typedef std::codecvt<wchar_t, char, std::mbstate_t> F;

int main()
{
    std::locale l = std::locale::classic();
    const std::basic_string<F::extern_type> from("some text");
    const std::basic_string<F::intern_type> expected(from.begin(), from.end());
    std::basic_string<F::intern_type> to(from.size(), F::intern_type());
    const F& f = std::use_facet<F>(l);
    std::mbstate_t mbs = {};
    const F::extern_type* from_next = 0;
    F::intern_type* to_next = 0;
    F::result r = f.in(mbs, from.data(), from.data() + from.size(), from_next,
                            &to[0], &to[0] + to.size(), to_next);
    assert(r == F::ok);
    assert(static_cast<std::size_t>(from_next - from.data()) == from.size());
    assert(static_cast<std::size_t>(to_next - to.data()) == expected.size());
    assert(static_cast<std::size_t>(to_next - to.data()) == expected.size());
    assert(to == expected);
}
