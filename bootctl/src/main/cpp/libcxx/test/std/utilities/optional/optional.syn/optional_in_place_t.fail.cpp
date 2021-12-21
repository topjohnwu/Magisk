//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// A program that necessitates the instantiation of template optional for
// (possibly cv-qualified) in_place_t is ill-formed.

#include <optional>

int main()
{
    using std::optional;
    using std::in_place_t;
    using std::in_place;

    optional<in_place_t> opt; // expected-note {{requested here}}
    // expected-error@optional:* {{"instantiation of optional with in_place_t is ill-formed"}}
}
