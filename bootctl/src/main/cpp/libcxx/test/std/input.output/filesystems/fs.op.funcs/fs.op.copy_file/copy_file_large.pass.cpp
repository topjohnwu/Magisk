//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// REQUIRES: long_tests

// <filesystem>

// bool copy_file(const path& from, const path& to);
// bool copy_file(const path& from, const path& to, error_code& ec) noexcept;
// bool copy_file(const path& from, const path& to, copy_options options);
// bool copy_file(const path& from, const path& to, copy_options options,
//           error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_copy_file_test_suite)

static std::string random_hex_chars(uintmax_t size) {
  std::string data;
  data.reserve(size);
  for (uintmax_t I = 0; I < size; ++I)
    data.push_back(random_utils::random_hex_char());
  return data;
}

// This test is intended to test 'sendfile's 2gb limit for a single call, and
// to ensure that libc++ correctly copies files larger than that limit.
// However it requires allocating ~5GB of filesystem space. This might not
// be acceptable on all systems.
TEST_CASE(large_file) {
  using namespace fs;
  constexpr uintmax_t sendfile_size_limit = 2147479552ull;
  constexpr uintmax_t additional_size = 1024;
  constexpr uintmax_t test_file_size = sendfile_size_limit + additional_size;
  static_assert(test_file_size > sendfile_size_limit, "");

  scoped_test_env env;

  // Check that we have more than sufficient room to create the files needed
  // to perform the test.
  if (space(env.test_root).available < 3 * test_file_size) {
    TEST_UNSUPPORTED();
  }

  // Use python to create a file right at the size limit.
  const path file = env.create_file("source", sendfile_size_limit);
  // Create some random data that looks different than the data before the
  // size limit.
  const std::string additional_data = random_hex_chars(additional_size);
  // Append this known data to the end of the source file.
  {
    std::ofstream outf(file.native(), std::ios_base::app);
    TEST_REQUIRE(outf.good());
    outf << additional_data;
    TEST_REQUIRE(outf);
  }
  TEST_REQUIRE(file_size(file) == test_file_size);
  const path dest = env.make_env_path("dest");

  std::error_code ec = GetTestEC();
  TEST_CHECK(copy_file(file, dest, ec));
  TEST_CHECK(!ec);

  TEST_REQUIRE(is_regular_file(dest));
  TEST_CHECK(file_size(dest) == test_file_size);

  // Read the data from the end of the destination file, and ensure it matches
  // the data at the end of the source file.
  std::string out_data;
  out_data.reserve(additional_size);
  {
    std::ifstream dest_file(dest.native());
    TEST_REQUIRE(dest_file);
    dest_file.seekg(sendfile_size_limit);
    TEST_REQUIRE(dest_file);
    dest_file >> out_data;
    TEST_CHECK(dest_file.eof());
  }
  TEST_CHECK(out_data.size() == additional_data.size());
  TEST_CHECK(out_data == additional_data);
}

TEST_SUITE_END()
