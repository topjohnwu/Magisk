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
// enum error_type
// {
//     error_collate    = unspecified,
//     error_ctype      = unspecified,
//     error_escape     = unspecified,
//     error_backref    = unspecified,
//     error_brack      = unspecified,
//     error_paren      = unspecified,
//     error_brace      = unspecified,
//     error_badbrace   = unspecified,
//     error_range      = unspecified,
//     error_space      = unspecified,
//     error_badrepeat  = unspecified,
//     error_complexity = unspecified,
//     error_stack      = unspecified
// };
//
// }

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    assert(std::regex_constants::error_collate != 0);
    assert(std::regex_constants::error_ctype != 0);
    assert(std::regex_constants::error_escape != 0);
    assert(std::regex_constants::error_backref != 0);
    assert(std::regex_constants::error_brack != 0);
    assert(std::regex_constants::error_paren != 0);
    assert(std::regex_constants::error_brace != 0);
    assert(std::regex_constants::error_badbrace != 0);
    assert(std::regex_constants::error_range != 0);
    assert(std::regex_constants::error_space != 0);
    assert(std::regex_constants::error_badrepeat != 0);
    assert(std::regex_constants::error_complexity != 0);
    assert(std::regex_constants::error_stack != 0);

    assert(std::regex_constants::error_collate != std::regex_constants::error_ctype);
    assert(std::regex_constants::error_collate != std::regex_constants::error_escape);
    assert(std::regex_constants::error_collate != std::regex_constants::error_backref);
    assert(std::regex_constants::error_collate != std::regex_constants::error_brack);
    assert(std::regex_constants::error_collate != std::regex_constants::error_paren);
    assert(std::regex_constants::error_collate != std::regex_constants::error_brace);
    assert(std::regex_constants::error_collate != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_collate != std::regex_constants::error_range);
    assert(std::regex_constants::error_collate != std::regex_constants::error_space);
    assert(std::regex_constants::error_collate != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_collate != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_collate != std::regex_constants::error_stack);

    assert(std::regex_constants::error_ctype != std::regex_constants::error_escape);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_backref);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_brack);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_paren);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_brace);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_range);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_space);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_ctype != std::regex_constants::error_stack);

    assert(std::regex_constants::error_escape != std::regex_constants::error_backref);
    assert(std::regex_constants::error_escape != std::regex_constants::error_brack);
    assert(std::regex_constants::error_escape != std::regex_constants::error_paren);
    assert(std::regex_constants::error_escape != std::regex_constants::error_brace);
    assert(std::regex_constants::error_escape != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_escape != std::regex_constants::error_range);
    assert(std::regex_constants::error_escape != std::regex_constants::error_space);
    assert(std::regex_constants::error_escape != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_escape != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_escape != std::regex_constants::error_stack);

    assert(std::regex_constants::error_backref != std::regex_constants::error_brack);
    assert(std::regex_constants::error_backref != std::regex_constants::error_paren);
    assert(std::regex_constants::error_backref != std::regex_constants::error_brace);
    assert(std::regex_constants::error_backref != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_backref != std::regex_constants::error_range);
    assert(std::regex_constants::error_backref != std::regex_constants::error_space);
    assert(std::regex_constants::error_backref != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_backref != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_backref != std::regex_constants::error_stack);

    assert(std::regex_constants::error_brack != std::regex_constants::error_paren);
    assert(std::regex_constants::error_brack != std::regex_constants::error_brace);
    assert(std::regex_constants::error_brack != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_brack != std::regex_constants::error_range);
    assert(std::regex_constants::error_brack != std::regex_constants::error_space);
    assert(std::regex_constants::error_brack != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_brack != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_brack != std::regex_constants::error_stack);

    assert(std::regex_constants::error_paren != std::regex_constants::error_brace);
    assert(std::regex_constants::error_paren != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_paren != std::regex_constants::error_range);
    assert(std::regex_constants::error_paren != std::regex_constants::error_space);
    assert(std::regex_constants::error_paren != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_paren != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_paren != std::regex_constants::error_stack);

    assert(std::regex_constants::error_brace != std::regex_constants::error_badbrace);
    assert(std::regex_constants::error_brace != std::regex_constants::error_range);
    assert(std::regex_constants::error_brace != std::regex_constants::error_space);
    assert(std::regex_constants::error_brace != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_brace != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_brace != std::regex_constants::error_stack);

    assert(std::regex_constants::error_badbrace != std::regex_constants::error_range);
    assert(std::regex_constants::error_badbrace != std::regex_constants::error_space);
    assert(std::regex_constants::error_badbrace != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_badbrace != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_badbrace != std::regex_constants::error_stack);

    assert(std::regex_constants::error_range != std::regex_constants::error_space);
    assert(std::regex_constants::error_range != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_range != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_range != std::regex_constants::error_stack);

    assert(std::regex_constants::error_space != std::regex_constants::error_badrepeat);
    assert(std::regex_constants::error_space != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_space != std::regex_constants::error_stack);

    assert(std::regex_constants::error_badrepeat != std::regex_constants::error_complexity);
    assert(std::regex_constants::error_badrepeat != std::regex_constants::error_stack);

    assert(std::regex_constants::error_complexity != std::regex_constants::error_stack);
}
