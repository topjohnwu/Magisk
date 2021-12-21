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

// bool status_known(file_status s) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(status_known_test_suite)

TEST_CASE(signature_test)
{
    file_status s; ((void)s);
    ASSERT_SAME_TYPE(decltype(status_known(s)), bool);
    ASSERT_NOEXCEPT(status_known(s));
}

TEST_CASE(status_known_test)
{
    struct TestCase {
        file_type type;
        bool expect;
    };
    const TestCase testCases[] = {
        {file_type::none, false},
        {file_type::not_found, true},
        {file_type::regular, true},
        {file_type::directory, true},
        {file_type::symlink, true},
        {file_type::block, true},
        {file_type::character, true},
        {file_type::fifo, true},
        {file_type::socket, true},
        {file_type::unknown, true}
    };
    for (auto& TC : testCases) {
        file_status s(TC.type);
        TEST_CHECK(status_known(s) == TC.expect);
    }
}

TEST_SUITE_END()
