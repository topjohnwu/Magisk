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

// directory_iterator(directory_iterator&&) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(directory_iterator_move_construct_tests)

TEST_CASE(test_constructor_signature)
{
    using D = directory_iterator;
    static_assert(std::is_nothrow_move_constructible<D>::value, "");
}

TEST_CASE(test_move_end_iterator)
{
    const directory_iterator endIt;
    directory_iterator endIt2{};

    directory_iterator it(std::move(endIt2));
    TEST_CHECK(it == endIt);
    TEST_CHECK(endIt2 == endIt);
}

TEST_CASE(test_move_valid_iterator)
{
    const path testDir = StaticEnv::Dir;
    const directory_iterator endIt{};

    directory_iterator it(testDir);
    TEST_REQUIRE(it != endIt);
    const path entry = *it;

    const directory_iterator it2(std::move(it));
    TEST_CHECK(*it2 == entry);

    TEST_CHECK(it == it2 || it == endIt);
}

TEST_SUITE_END()
