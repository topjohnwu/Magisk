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

// path temp_directory_path();
// path temp_directory_path(error_code& ec);

#include "filesystem_include.hpp"
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

void PutEnv(std::string var, std::string value) {
    assert(::setenv(var.c_str(), value.c_str(), /* overwrite */ 1) == 0);
}

void UnsetEnv(std::string var) {
    assert(::unsetenv(var.c_str()) == 0);
}

TEST_SUITE(filesystem_temp_directory_path_test_suite)

TEST_CASE(signature_test)
{
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(temp_directory_path());
    ASSERT_NOT_NOEXCEPT(temp_directory_path(ec));
}

TEST_CASE(basic_tests)
{
    scoped_test_env env;
    const path dne = env.make_env_path("dne");
    const path file = env.create_file("file", 42);
    const path dir_perms = env.create_dir("bad_perms_dir");
    const path nested_dir = env.create_dir("bad_perms_dir/nested");
    permissions(dir_perms, perms::none);
    const std::error_code expect_ec = std::make_error_code(std::errc::not_a_directory);
    struct TestCase {
      std::string name;
      path p;
    } cases[] = {
        {"TMPDIR", env.create_dir("dir1")},
        {"TMP", env.create_dir("dir2")},
        {"TEMP", env.create_dir("dir3")},
        {"TEMPDIR", env.create_dir("dir4")}
    };
    for (auto& TC : cases) {
        PutEnv(TC.name, TC.p);
    }
    for (auto& TC : cases) {
        std::error_code ec = GetTestEC();
        path ret = temp_directory_path(ec);
        TEST_CHECK(!ec);
        TEST_CHECK(ret == TC.p);
        TEST_CHECK(is_directory(ret));

        // Set the env variable to a path that does not exist and check
        // that it fails.
        PutEnv(TC.name, dne);
        ec = GetTestEC();
        ret = temp_directory_path(ec);
        LIBCPP_ONLY(TEST_CHECK(ec == expect_ec));
        TEST_CHECK(ec != GetTestEC());
        TEST_CHECK(ec);
        TEST_CHECK(ret == "");

        // Set the env variable to point to a file and check that it fails.
        PutEnv(TC.name, file);
        ec = GetTestEC();
        ret = temp_directory_path(ec);
        LIBCPP_ONLY(TEST_CHECK(ec == expect_ec));
        TEST_CHECK(ec != GetTestEC());
        TEST_CHECK(ec);
        TEST_CHECK(ret == "");

        // Set the env variable to point to a dir we can't access
        PutEnv(TC.name, nested_dir);
        ec = GetTestEC();
        ret = temp_directory_path(ec);
        TEST_CHECK(ec == std::make_error_code(std::errc::permission_denied));
        TEST_CHECK(ret == "");

        // Set the env variable to point to a non-existent dir
        PutEnv(TC.name, TC.p / "does_not_exist");
        ec = GetTestEC();
        ret = temp_directory_path(ec);
        TEST_CHECK(ec != GetTestEC());
        TEST_CHECK(ec);
        TEST_CHECK(ret == "");

        // Finally erase this env variable
        UnsetEnv(TC.name);
    }
    // No env variables are defined
    {
        std::error_code ec = GetTestEC();
        path ret = temp_directory_path(ec);
#if defined(__ANDROID__)
        // Android has no globally writable storage and thus this API does not
        // work without environment configuration.
        TEST_CHECK(ec != GetTestEC());
        TEST_CHECK(ec);
        TEST_CHECK(ret == "");
#else
        TEST_CHECK(!ec);
        TEST_CHECK(ret == "/tmp");
        TEST_CHECK(is_directory(ret));
#endif
    }
}

TEST_SUITE_END()
