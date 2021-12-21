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

// class directory_entry

// file_status symlink_status() const;
// file_status symlink_status(error_code&) const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "filesystem_test_helper.hpp"
#include "rapid-cxx-test.hpp"

TEST_SUITE(directory_entry_obs_suite)

TEST_CASE(test_signature) {
  using namespace fs;
  {
    const directory_entry e("foo");
    std::error_code ec;
    static_assert(std::is_same<decltype(e.symlink_status()), file_status>::value, "");
    static_assert(std::is_same<decltype(e.symlink_status(ec)), file_status>::value, "");
    static_assert(noexcept(e.symlink_status()) == false, "");
    static_assert(noexcept(e.symlink_status(ec)) == true, "");
  }
  path TestCases[] = {StaticEnv::File, StaticEnv::Dir, StaticEnv::SymlinkToFile,
                      StaticEnv::DNE};
  for (const auto& p : TestCases) {
    const directory_entry e(p);
    std::error_code pec = GetTestEC(), eec = GetTestEC(1);
    file_status ps = fs::symlink_status(p, pec);
    file_status es = e.symlink_status(eec);
    TEST_CHECK(ps.type() == es.type());
    TEST_CHECK(ps.permissions() == es.permissions());
    TEST_CHECK(pec == eec);
  }
  for (const auto& p : TestCases) {
    const directory_entry e(p);
    file_status ps = fs::symlink_status(p);
    file_status es = e.symlink_status();
    TEST_CHECK(ps.type() == es.type());
    TEST_CHECK(ps.permissions() == es.permissions());
  }
}

TEST_SUITE_END()
