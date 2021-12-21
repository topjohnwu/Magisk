// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// namespace regex_constants
// {
//
// emum match_flag_type  // bitmask type
// {
//     match_default     = 0,
//     match_not_bol     = unspecified,
//     match_not_eol     = unspecified,
//     match_not_bow     = unspecified,
//     match_not_eow     = unspecified,
//     match_any         = unspecified,
//     match_not_null    = unspecified,
//     match_continuous  = unspecified,
//     match_prev_avail  = unspecified,
//     format_default    = 0,
//     format_sed        = unspecified,
//     format_no_copy    = unspecified,
//     format_first_only = unspecified
// };
//
// }

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    assert(std::regex_constants::match_default == 0);
    assert(std::regex_constants::match_not_bol != 0);
    assert(std::regex_constants::match_not_eol != 0);
    assert(std::regex_constants::match_not_bow != 0);
    assert(std::regex_constants::match_not_eow != 0);
    assert(std::regex_constants::match_any != 0);
    assert(std::regex_constants::match_not_null != 0);
    assert(std::regex_constants::match_continuous != 0);
    assert(std::regex_constants::match_prev_avail != 0);
    assert(std::regex_constants::format_default == 0);
    assert(std::regex_constants::format_sed != 0);
    assert(std::regex_constants::format_no_copy != 0);
    assert(std::regex_constants::format_first_only != 0);

    assert((std::regex_constants::match_not_bol & std::regex_constants::match_not_eol) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_not_bow) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_not_eow) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_any) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_not_null) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_not_bol & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_not_eol & std::regex_constants::match_not_bow) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::match_not_eow) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::match_any) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::match_not_null) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_not_eol & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_not_bow & std::regex_constants::match_not_eow) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::match_any) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::match_not_null) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_not_bow & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_not_eow & std::regex_constants::match_any) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::match_not_null) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_not_eow & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_any & std::regex_constants::match_not_null) == 0);
    assert((std::regex_constants::match_any & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_any & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_any & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_any & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_any & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_not_null & std::regex_constants::match_continuous) == 0);
    assert((std::regex_constants::match_not_null & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_not_null & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_not_null & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_not_null & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_continuous & std::regex_constants::match_prev_avail) == 0);
    assert((std::regex_constants::match_continuous & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_continuous & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_continuous & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::match_prev_avail & std::regex_constants::format_sed) == 0);
    assert((std::regex_constants::match_prev_avail & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::match_prev_avail & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::format_sed & std::regex_constants::format_no_copy) == 0);
    assert((std::regex_constants::format_sed & std::regex_constants::format_first_only) == 0);

    assert((std::regex_constants::format_no_copy & std::regex_constants::format_first_only) == 0);

    std::regex_constants::match_flag_type e1 = std::regex_constants::match_not_bol;
    std::regex_constants::match_flag_type e2 = std::regex_constants::match_not_eol;
    e1 = ~e1;
    e1 = e1 & e2;
    e1 = e1 | e2;
    e1 = e1 ^ e2;
    e1 &= e2;
    e1 |= e2;
    e1 ^= e2;
}
