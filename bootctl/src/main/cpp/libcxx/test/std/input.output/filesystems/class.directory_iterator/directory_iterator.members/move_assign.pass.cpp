//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class directory_iterator

// directory_iterator& operator=(directory_iterator const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

// The filesystem specification explicitly allows for self-move on
// the directory iterators. Turn off this warning so we can test it.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wself-move"
#endif

using namespace fs;

TEST_SUITE(directory_iterator_move_assign_tests)

TEST_CASE(test_assignment_signature)
{
    using D = directory_iterator;
    static_assert(std::is_nothrow_move_assignable<D>::value, "");
}

TEST_CASE(test_move_to_end_iterator)
{
    const path testDir = StaticEnv::Dir;

    directory_iterator from(testDir);
    TEST_REQUIRE(from != directory_iterator{});
    const path entry = *from;

    directory_iterator to{};
    to = std::move(from);
    TEST_REQUIRE(to != directory_iterator{});
    TEST_CHECK(*to == entry);
}


TEST_CASE(test_move_from_end_iterator)
{
    const path testDir = StaticEnv::Dir;

    directory_iterator from{};

    directory_iterator to(testDir);
    TEST_REQUIRE(to != from);

    to = std::move(from);
    TEST_REQUIRE(to == directory_iterator{});
    TEST_REQUIRE(from == directory_iterator{});
}

TEST_CASE(test_move_valid_iterator)
{
    const path testDir = StaticEnv::Dir;
    const directory_iterator endIt{};

    directory_iterator it(testDir);
    TEST_REQUIRE(it != endIt);
    ++it;
    TEST_REQUIRE(it != endIt);
    const path entry = *it;

    directory_iterator it2(testDir);
    TEST_REQUIRE(it2 != it);
    const path entry2 = *it2;
    TEST_CHECK(entry2 != entry);

    it2 = std::move(it);
    TEST_REQUIRE(it2 != directory_iterator{});
    TEST_CHECK(*it2 == entry);
}

TEST_CASE(test_returns_reference_to_self)
{
    directory_iterator it;
    directory_iterator it2;
    directory_iterator& ref = (it2 = it);
    TEST_CHECK(&ref == &it2);
}


TEST_CASE(test_self_move)
{
    // Create two non-equal iterators that have exactly the same state.
    directory_iterator it(StaticEnv::Dir);
    directory_iterator it2(StaticEnv::Dir);
    ++it; ++it2;
    TEST_CHECK(it != it2);
    TEST_CHECK(*it2 == *it);

    it = std::move(it);
    TEST_CHECK(*it2 == *it);
}


TEST_SUITE_END()
