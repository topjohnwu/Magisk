//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_token_iterator<BidirectionalIterator, charT, traits>

// bool operator==(const regex_token_iterator& right) const;
// bool operator!=(const regex_token_iterator& right) const;

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        const char phone_book[] = "start 555-1234, 555-2345, 555-3456 end";
        std::cregex_token_iterator i(std::begin(phone_book), std::end(phone_book)-1,
                                     phone_numbers, -1);
        assert(i != std::cregex_token_iterator());
        assert(!(i == std::cregex_token_iterator()));
        std::cregex_token_iterator i2 = i;
        assert(i2 == i);
        assert(!(i2 != i));
        ++i;
        assert(!(i2 == i));
        assert(i2 != i);
    }
}
