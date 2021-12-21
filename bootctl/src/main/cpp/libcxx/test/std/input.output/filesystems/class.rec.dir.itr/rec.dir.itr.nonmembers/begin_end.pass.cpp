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

// recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept;
// recursive_directory_iterator end(recursive_directory_iterator iter) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"
#include <iostream>

using namespace fs;

TEST_SUITE(recursive_directory_iterator_begin_end_tests)

TEST_CASE(test_function_signatures)
{
    recursive_directory_iterator d; ((void)d);

    ASSERT_SAME_TYPE(decltype(begin(d)), recursive_directory_iterator);
    ASSERT_NOEXCEPT(begin(std::move(d)));

    ASSERT_SAME_TYPE(decltype(end(d)), recursive_directory_iterator);
    ASSERT_NOEXCEPT(end(std::move(d)));
}

TEST_CASE(test_ranged_for_loop)
{
    const path testDir = StaticEnv::Dir;
    std::set<path> dir_contents(std::begin(StaticEnv::RecDirIterationList),
                                std::end(  StaticEnv::RecDirIterationList));

    std::error_code ec;
    recursive_directory_iterator it(testDir, ec);
    TEST_REQUIRE(!ec);

    for (auto& elem : it) {
        TEST_CHECK(dir_contents.erase(elem) == 1);
    }
    TEST_CHECK(dir_contents.empty());
}

TEST_SUITE_END()
