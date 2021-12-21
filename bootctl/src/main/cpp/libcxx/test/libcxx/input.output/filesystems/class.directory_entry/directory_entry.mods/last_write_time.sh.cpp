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

// RUN: %build -I%libcxx_src_root/src/filesystem
// RUN: %run

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

#include "filesystem_common.h"

using namespace fs::detail;

TEST_SUITE(directory_entry_mods_suite)

TEST_CASE(last_write_time_not_representable_error) {
  using namespace fs;
  using namespace std::chrono;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);

  TimeSpec ToTime;
  ToTime.tv_sec = std::numeric_limits<decltype(ToTime.tv_sec)>::max();
  ToTime.tv_nsec = duration_cast<nanoseconds>(seconds(1)).count() - 1;

  std::array<TimeSpec, 2> TS = {ToTime, ToTime};

  file_time_type old_time = last_write_time(file);
  directory_entry ent(file);

  file_time_type start_time = file_time_type::clock::now() - hours(1);
  last_write_time(file, start_time);

  TEST_CHECK(ent.last_write_time() == old_time);

  bool IsRepresentable = true;
  file_time_type rep_value;
  {
    std::error_code ec;
    TEST_REQUIRE(!set_file_times(file, TS, ec));
    ec.clear();
    rep_value = last_write_time(file, ec);
    IsRepresentable = !bool(ec);
  }

  if (!IsRepresentable) {
    std::error_code rec = GetTestEC();
    ent.refresh(rec);
    TEST_CHECK(!rec);

    const std::errc expected_err = std::errc::value_too_large;

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, expected_err));

    ec = GetTestEC();
    TEST_CHECK(last_write_time(file, ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, expected_err));

    ExceptionChecker CheckExcept(file, expected_err,
                                 "directory_entry::last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, CheckExcept,
                            ent.last_write_time());

  } else {
    ent.refresh();

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == rep_value);
    TEST_CHECK(!ec);
  }
}

TEST_SUITE_END()
