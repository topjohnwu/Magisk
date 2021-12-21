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

// directory_iterator begin(directory_iterator iter) noexcept;
// directory_iterator end(directory_iterator iter) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"
#include <iostream>

using namespace fs;

TEST_SUITE(directory_iterator_begin_end_tests)

TEST_CASE(test_function_signatures)
{
    directory_iterator d; ((void)d);

    ASSERT_SAME_TYPE(decltype(begin(d)), directory_iterator);
    ASSERT_NOEXCEPT(begin(std::move(d)));

    ASSERT_SAME_TYPE(decltype(end(d)), directory_iterator);
    ASSERT_NOEXCEPT(end(std::move(d)));
}

TEST_CASE(test_ranged_for_loop)
{
    const path testDir = StaticEnv::Dir;
    std::set<path> dir_contents(std::begin(StaticEnv::DirIterationList),
                                      std::end(  StaticEnv::DirIterationList));

    std::error_code ec;
    directory_iterator it(testDir, ec);
    TEST_REQUIRE(!ec);

    for (auto& elem : it) {
        TEST_CHECK(dir_contents.erase(elem) == 1);
    }
    TEST_CHECK(dir_contents.empty());
}

TEST_SUITE_END()
