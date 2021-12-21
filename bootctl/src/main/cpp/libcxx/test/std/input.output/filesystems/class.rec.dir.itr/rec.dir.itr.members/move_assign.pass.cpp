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

// class recursive_directory_iterator

// recursive_directory_iterator& operator=(recursive_directory_iterator const&);

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

TEST_SUITE(recursive_directory_iterator_move_assign_tests)

recursive_directory_iterator createInterestingIterator()
    // Create an "interesting" iterator where all fields are
    // in a non-default state. The returned 'it' is in a
    // state such that:
    //   it.options() == directory_options::skip_permission_denied
    //   it.depth() == 1
    //   it.recursion_pending() == true
{
    const path testDir = StaticEnv::Dir;
    const recursive_directory_iterator endIt;
    recursive_directory_iterator it(testDir,
                                    directory_options::skip_permission_denied);
    TEST_ASSERT(it != endIt);
    while (it.depth() != 1) {
        ++it;
        TEST_ASSERT(it != endIt);
    }
    TEST_ASSERT(it.depth() == 1);
    it.disable_recursion_pending();
    return it;
}

recursive_directory_iterator createDifferentInterestingIterator()
    // Create an "interesting" iterator where all fields are
    // in a non-default state. The returned 'it' is in a
    // state such that:
    //   it.options() == directory_options::follow_directory_symlink
    //   it.depth() == 2
    //   it.recursion_pending() == false
{
    const path testDir = StaticEnv::Dir;
    const recursive_directory_iterator endIt;
    recursive_directory_iterator it(testDir,
                                    directory_options::follow_directory_symlink);
    TEST_ASSERT(it != endIt);
    while (it.depth() != 2) {
        ++it;
        TEST_ASSERT(it != endIt);
    }
    TEST_ASSERT(it.depth() == 2);
    return it;
}


TEST_CASE(test_assignment_signature)
{
    using D = recursive_directory_iterator;
    static_assert(std::is_nothrow_move_assignable<D>::value, "");
}


TEST_CASE(test_move_to_end_iterator)
{
    const recursive_directory_iterator endIt;

    recursive_directory_iterator from = createInterestingIterator();
    const recursive_directory_iterator from_copy(from);
    const path entry = *from;

    recursive_directory_iterator to;
    to = std::move(from);
    TEST_REQUIRE(to != endIt);
    TEST_CHECK(*to == entry);
    TEST_CHECK(to.options() == from_copy.options());
    TEST_CHECK(to.depth() == from_copy.depth());
    TEST_CHECK(to.recursion_pending() == from_copy.recursion_pending());
    TEST_CHECK(from == endIt || from == to);
}


TEST_CASE(test_move_from_end_iterator)
{
    recursive_directory_iterator from;
    recursive_directory_iterator to = createInterestingIterator();

    to = std::move(from);
    TEST_REQUIRE(to == from);
    TEST_CHECK(to == recursive_directory_iterator{});
}

TEST_CASE(test_move_valid_iterator)
{
    const recursive_directory_iterator endIt;

    recursive_directory_iterator it = createInterestingIterator();
    const recursive_directory_iterator it_copy(it);
    const path entry = *it;

    recursive_directory_iterator it2 = createDifferentInterestingIterator();
    const recursive_directory_iterator it2_copy(it2);
    TEST_REQUIRE(it2 != it);
    TEST_CHECK(it2.options() != it.options());
    TEST_CHECK(it2.depth() != it.depth());
    TEST_CHECK(it2.recursion_pending() != it.recursion_pending());
    TEST_CHECK(*it2 != entry);

    it2 = std::move(it);
    TEST_REQUIRE(it2 != it2_copy && it2 != endIt);
    TEST_CHECK(it2.options() == it_copy.options());
    TEST_CHECK(it2.depth() == it_copy.depth());
    TEST_CHECK(it2.recursion_pending() == it_copy.recursion_pending());
    TEST_CHECK(*it2 == entry);
    TEST_CHECK(it == endIt || it == it2);
}

TEST_CASE(test_returns_reference_to_self)
{
    recursive_directory_iterator it;
    recursive_directory_iterator it2;
    recursive_directory_iterator& ref = (it2 = std::move(it));
    TEST_CHECK(&ref == &it2);
}

TEST_CASE(test_self_move)
{
    // Create two non-equal iterators that have exactly the same state.
    recursive_directory_iterator it = createInterestingIterator();
    recursive_directory_iterator it2 = createInterestingIterator();
    TEST_CHECK(it != it2);
    TEST_CHECK(it2.options()           == it.options());
    TEST_CHECK(it2.depth()             == it.depth());
    TEST_CHECK(it2.recursion_pending() == it.recursion_pending());
    TEST_CHECK(*it2 == *it);

    it = std::move(it);
    TEST_CHECK(it2.options()           == it.options());
    TEST_CHECK(it2.depth()             == it.depth());
    TEST_CHECK(it2.recursion_pending() == it.recursion_pending());
    TEST_CHECK(*it2 == *it);
}


TEST_SUITE_END()
