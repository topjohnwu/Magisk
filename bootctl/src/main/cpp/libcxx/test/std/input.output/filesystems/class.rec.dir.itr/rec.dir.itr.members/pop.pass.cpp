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

// void pop();
// void pop(error_code& ec);

#include "filesystem_include.hpp"
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(recursive_directory_iterator_pop_tests)

TEST_CASE(signature_tests)
{
    recursive_directory_iterator it{}; ((void)it);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(it.pop());
    ASSERT_NOT_NOEXCEPT(it.pop(ec)); // may require allocation or other things
}

// NOTE: Since the order of iteration is unspecified we use a list of
// seen files at each depth to determine the new depth after a 'pop()' operation.
TEST_CASE(test_depth)
{
    const recursive_directory_iterator endIt{};

    auto& DE0 = StaticEnv::DirIterationList;
    std::set<path> notSeenDepth0(std::begin(DE0), std::end(DE0));

    auto& DE1 = StaticEnv::DirIterationListDepth1;
    std::set<path> notSeenDepth1(std::begin(DE1), std::end(DE1));

    std::error_code ec;
    recursive_directory_iterator it(StaticEnv::Dir, ec);
    TEST_REQUIRE(it != endIt);
    TEST_CHECK(it.depth() == 0);

    while (it.depth() != 2) {
        if (it.depth() == 0)
            notSeenDepth0.erase(it->path());
        else
            notSeenDepth1.erase(it->path());
        ++it;
        TEST_REQUIRE(it != endIt);
    }

    while (true) {
        auto set_ec = std::make_error_code(std::errc::address_in_use);
        it.pop(set_ec);
        TEST_REQUIRE(!set_ec);

        if (it == endIt) {
            // We must have seen every entry at depth 0 and 1.
            TEST_REQUIRE(notSeenDepth0.empty() && notSeenDepth1.empty());
            break;
        }
        else if (it.depth() == 1) {
            // If we popped to depth 1 then there must be unseen entries
            // at this level.
            TEST_REQUIRE(!notSeenDepth1.empty());
            TEST_CHECK(notSeenDepth1.count(it->path()));
            notSeenDepth1.clear();
        }
        else if (it.depth() == 0) {
            // If we popped to depth 0 there must be unseen entries at this
            // level. There should also be no unseen entries at depth 1.
            TEST_REQUIRE(!notSeenDepth0.empty());
            TEST_REQUIRE(notSeenDepth1.empty());
            TEST_CHECK(notSeenDepth0.count(it->path()));
            notSeenDepth0.clear();
        }
    }
}

TEST_SUITE_END()
