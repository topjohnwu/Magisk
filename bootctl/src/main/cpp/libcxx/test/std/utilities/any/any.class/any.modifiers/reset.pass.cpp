//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <any>

// any::reset() noexcept

#include <any>
#include <cassert>

#include "any_helpers.h"

int main()
{
    using std::any;
    using std::any_cast;
    // empty
    {
        any a;

        // noexcept check
        static_assert(
            noexcept(a.reset())
          , "any.reset() must be noexcept"
          );

        assertEmpty(a);

        a.reset();

        assertEmpty(a);
    }
    // small object
    {
        any a((small(1)));
        assert(small::count == 1);
        assertContains<small>(a, 1);

        a.reset();

        assertEmpty<small>(a);
        assert(small::count == 0);
    }
    // large object
    {
        any a(large(1));
        assert(large::count == 1);
        assertContains<large>(a, 1);

        a.reset();

        assertEmpty<large>(a);
        assert(large::count == 0);
    }
}
