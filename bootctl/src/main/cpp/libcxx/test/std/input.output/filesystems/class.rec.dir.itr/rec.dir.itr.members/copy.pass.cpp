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

// recursive_recursive_directory_iterator(recursive_recursive_directory_iterator const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(recursive_directory_iterator_copy_construct_tests)

TEST_CASE(test_constructor_signature)
{
    using D = recursive_directory_iterator;
    static_assert(std::is_copy_constructible<D>::value, "");
    //static_assert(!std::is_nothrow_copy_constructible<D>::value, "");
}

TEST_CASE(test_copy_end_iterator)
{
    const recursive_directory_iterator endIt;
    recursive_directory_iterator it(endIt);
    TEST_CHECK(it == endIt);
}

TEST_CASE(test_copy_valid_iterator)
{
    const path testDir = StaticEnv::Dir;
    const recursive_directory_iterator endIt{};

    // build 'it' up with "interesting" non-default state so we can test
    // that it gets copied. We want to get 'it' into a state such that:
    //  it.options() != directory_options::none
    //  it.depth() != 0
    //  it.recursion_pending() != true
    const directory_options opts = directory_options::skip_permission_denied;
    recursive_directory_iterator it(testDir, opts);
    TEST_REQUIRE(it != endIt);
    while (it.depth() == 0) {
        ++it;
        TEST_REQUIRE(it != endIt);
    }
    it.disable_recursion_pending();
    TEST_CHECK(it.options() == opts);
    TEST_CHECK(it.depth() == 1);
    TEST_CHECK(it.recursion_pending() == false);
    const path entry = *it;

    // OPERATION UNDER TEST //
    const recursive_directory_iterator it2(it);
    // ------------------- //

    TEST_REQUIRE(it2 == it);
    TEST_CHECK(*it2 == entry);
    TEST_CHECK(it2.depth() == 1);
    TEST_CHECK(it2.recursion_pending() == false);
    TEST_CHECK(it != endIt);
}

TEST_SUITE_END()
