//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <>
// class codecvt<char, char, mbstate_t>
//     : public locale::facet,
//       public codecvt_base
// {
// public:
//     typedef char      intern_type;
//     typedef char      extern_type;
//     typedef mbstate_t state_type;
//     ...
// };

#include <locale>
#include <type_traits>
#include <cassert>

int main()
{
    typedef std::codecvt<char, char, std::mbstate_t> F;
    static_assert((std::is_base_of<std::locale::facet, F>::value), "");
    static_assert((std::is_base_of<std::codecvt_base, F>::value), "");
    static_assert((std::is_same<F::intern_type, char>::value), "");
    static_assert((std::is_same<F::extern_type, char>::value), "");
    static_assert((std::is_same<F::state_type, std::mbstate_t>::value), "");
    std::locale l = std::locale::classic();
    assert(std::has_facet<F>(l));
    const F& f = std::use_facet<F>(l);
    ((void)f); // Prevent unused warning
    (void)F::id;
}
