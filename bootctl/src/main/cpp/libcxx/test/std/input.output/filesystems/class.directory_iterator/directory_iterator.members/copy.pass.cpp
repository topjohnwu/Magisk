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

// directory_iterator(directory_iterator const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(directory_iterator_copy_construct_tests)

TEST_CASE(test_constructor_signature)
{
    using D = directory_iterator;
    static_assert(std::is_copy_constructible<D>::value, "");
}

TEST_CASE(test_copy_end_iterator)
{
    const directory_iterator endIt;
    directory_iterator it(endIt);
    TEST_CHECK(it == endIt);
}

TEST_CASE(test_copy_valid_iterator)
{
    const path testDir = StaticEnv::Dir;
    const directory_iterator endIt{};

    const directory_iterator it(testDir);
    TEST_REQUIRE(it != endIt);
    const path entry = *it;

    const directory_iterator it2(it);
    TEST_REQUIRE(it2 == it);
    TEST_CHECK(*it2 == entry);
    TEST_CHECK(*it == entry);
}

TEST_SUITE_END()
