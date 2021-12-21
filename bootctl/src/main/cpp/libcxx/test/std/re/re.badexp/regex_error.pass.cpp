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

// class regex_error
//     : public runtime_error
// {
// public:
//     explicit regex_error(regex_constants::error_type ecode);
//     regex_constants::error_type code() const;
// };

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        std::regex_error e(std::regex_constants::error_collate);
        assert(e.code() == std::regex_constants::error_collate);
        assert(e.what() == std::string("The expression contained an invalid collating element name."));
    }
    {
        std::regex_error e(std::regex_constants::error_ctype);
        assert(e.code() == std::regex_constants::error_ctype);
        assert(e.what() == std::string("The expression contained an invalid character class name."));
    }
    {
        std::regex_error e(std::regex_constants::error_escape);
        assert(e.code() == std::regex_constants::error_escape);
        assert(e.what() == std::string("The expression contained an invalid escaped character, or a "
               "trailing escape."));
    }
    {
        std::regex_error e(std::regex_constants::error_backref);
        assert(e.code() == std::regex_constants::error_backref);
        assert(e.what() == std::string("The expression contained an invalid back reference."));
    }
    {
        std::regex_error e(std::regex_constants::error_brack);
        assert(e.code() == std::regex_constants::error_brack);
        assert(e.what() == std::string("The expression contained mismatched [ and ]."));
    }
    {
        std::regex_error e(std::regex_constants::error_paren);
        assert(e.code() == std::regex_constants::error_paren);
        assert(e.what() == std::string("The expression contained mismatched ( and )."));
    }
    {
        std::regex_error e(std::regex_constants::error_brace);
        assert(e.code() == std::regex_constants::error_brace);
        assert(e.what() == std::string("The expression contained mismatched { and }."));
    }
    {
        std::regex_error e(std::regex_constants::error_badbrace);
        assert(e.code() == std::regex_constants::error_badbrace);
        assert(e.what() == std::string("The expression contained an invalid range in a {} expression."));
    }
    {
        std::regex_error e(std::regex_constants::error_range);
        assert(e.code() == std::regex_constants::error_range);
        assert(e.what() == std::string("The expression contained an invalid character range, "
               "such as [b-a] in most encodings."));
    }
    {
        std::regex_error e(std::regex_constants::error_space);
        assert(e.code() == std::regex_constants::error_space);
        assert(e.what() == std::string("There was insufficient memory to convert the expression into "
               "a finite state machine."));
    }
    {
        std::regex_error e(std::regex_constants::error_badrepeat);
        assert(e.code() == std::regex_constants::error_badrepeat);
        assert(e.what() == std::string("One of *?+{ was not preceded by a valid regular expression."));
    }
    {
        std::regex_error e(std::regex_constants::error_complexity);
        assert(e.code() == std::regex_constants::error_complexity);
        assert(e.what() == std::string("The complexity of an attempted match against a regular "
               "expression exceeded a pre-set level."));
    }
    {
        std::regex_error e(std::regex_constants::error_stack);
        assert(e.code() == std::regex_constants::error_stack);
        assert(e.what() == std::string("There was insufficient memory to determine whether the regular "
               "expression could match the specified character sequence."));
    }
}
