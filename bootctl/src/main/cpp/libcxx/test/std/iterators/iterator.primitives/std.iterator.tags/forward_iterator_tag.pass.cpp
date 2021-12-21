//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// struct forward_iterator_tag: public input_iterator_tag {};

#include <iterator>
#include <type_traits>

int main()
{
    std::forward_iterator_tag tag;
    ((void)tag); // Prevent unused warning
    static_assert((std::is_base_of<std::input_iterator_tag,
                                   std::forward_iterator_tag>::value), "");
    static_assert((!std::is_base_of<std::output_iterator_tag,
                                    std::forward_iterator_tag>::value), "");
}
