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
// class ctype<char>
//     : public locale::facet,
//       public ctype_base
// {
// public:
//     typedef char char_type;
// };

#include <locale>
#include <type_traits>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        assert(std::has_facet<std::ctype<char> >(l));
        const std::ctype<char>& f = std::use_facet<std::ctype<char> >(l);
        ((void)f); // Prevent unused warning
        {
            (void)std::ctype<char>::id;
        }
        static_assert((std::is_same<std::ctype<char>::char_type, char>::value), "");
        static_assert((std::is_base_of<std::ctype_base, std::ctype<char> >::value), "");
        static_assert((std::is_base_of<std::locale::facet, std::ctype<char> >::value), "");
    }
}
