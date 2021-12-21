//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <filesystem>

// typedef TrivialClock file_time_type;

// RUN: %build -I%libcxx_src_root/src/filesystem
// RUN: %run

#include <filesystem>
#include <chrono>
#include <type_traits>
#include <limits>
#include <cstddef>
#include <cassert>

#include "filesystem_common.h"

#ifndef __SIZEOF_INT128__
#define TEST_HAS_NO_INT128_T
#endif

using namespace std::chrono;
namespace fs = std::__fs::filesystem;
using fs::file_time_type;
using fs::detail::time_util;

#ifdef TEST_HAS_NO_INT128_T
static_assert(sizeof(fs::file_time_type::rep) <= 8, "");
#endif

enum TestKind { TK_128Bit, TK_64Bit, TK_32Bit, TK_FloatingPoint };

template <class TimeT>
constexpr TestKind getTimeTTestKind() {
  if (sizeof(TimeT) == 8 && !std::is_floating_point<TimeT>::value)
    return TK_64Bit;
  else if (sizeof(TimeT) == 4 && !std::is_floating_point<TimeT>::value)
    return TK_32Bit;
  else if (std::is_floating_point<TimeT>::value)
    return TK_FloatingPoint;
  else
    assert(false && "test kind not supported");
}
template <class FileTimeT>
constexpr TestKind getFileTimeTestKind() {
  using Rep = typename FileTimeT::rep;
  if (std::is_floating_point<Rep>::value)
    return TK_FloatingPoint;
  else if (sizeof(Rep) == 16)
    return TK_128Bit;
  else if (sizeof(Rep) == 8)
    return TK_64Bit;
  else
    assert(false && "test kind not supported");
}

template <class FileTimeT, class TimeT, class TimeSpecT,
          class Base = time_util<FileTimeT, TimeT, TimeSpecT>,
          TestKind = getTimeTTestKind<TimeT>(),
          TestKind = getFileTimeTestKind<FileTimeT>()>
struct test_case;

template <class FileTimeT, class TimeT, class TimeSpecT, class Base>
struct test_case<FileTimeT, TimeT, TimeSpecT, Base, TK_64Bit, TK_128Bit>
    : public Base {

  using Base::convert_from_timespec;
  using Base::convert_to_timespec;
  using Base::is_representable;
  using Base::max_nsec;
  using Base::max_seconds;
  using Base::min_nsec_timespec;
  using Base::min_seconds;

  static constexpr auto max_time_t = std::numeric_limits<TimeT>::max();
  static constexpr auto min_time_t = std::numeric_limits<TimeT>::min();

  static constexpr bool test_timespec() {
    static_assert(is_representable(TimeSpecT{max_time_t, 0}), "");
    static_assert(is_representable(TimeSpecT{max_time_t, 999999999}), "");
    static_assert(is_representable(TimeSpecT{max_time_t, 1000000000}), "");
    static_assert(is_representable(TimeSpecT{max_time_t, max_nsec}), "");

    static_assert(is_representable(TimeSpecT{min_time_t, 0}), "");
    static_assert(is_representable(TimeSpecT{min_time_t, 999999999}), "");
    static_assert(is_representable(TimeSpecT{min_time_t, 1000000000}), "");
    static_assert(is_representable(TimeSpecT{min_time_t, min_nsec_timespec}),
                  "");

    return true;
  }

  static constexpr bool test_file_time_type() {
    // This kinda sucks. Oh well.
    static_assert(!Base::is_representable(FileTimeT::max()), "");
    static_assert(!Base::is_representable(FileTimeT::min()), "");
    return true;
  }

  static constexpr bool check_round_trip(TimeSpecT orig) {
    TimeSpecT new_ts = {};
    FileTimeT out = convert_from_timespec(orig);
    assert(convert_to_timespec(new_ts, out));
    return new_ts.tv_sec == orig.tv_sec && new_ts.tv_nsec == orig.tv_nsec;
  }

  static constexpr bool test_convert_timespec() {
    static_assert(check_round_trip({0, 0}), "");
    static_assert(check_round_trip({0, 1}), "");
    static_assert(check_round_trip({1, 1}), "");
    static_assert(check_round_trip({-1, 1}), "");
    static_assert(check_round_trip({max_time_t, max_nsec}), "");
    static_assert(check_round_trip({max_time_t, 123}), "");
    static_assert(check_round_trip({min_time_t, min_nsec_timespec}), "");
    static_assert(check_round_trip({min_time_t, 123}), "");
    return true;
  }

  static bool test() {
    static_assert(test_timespec(), "");
    static_assert(test_file_time_type(), "");
    static_assert(test_convert_timespec(), "");
    return true;
  }
};

template <class FileTimeT, class TimeT, class TimeSpecT, class Base>
struct test_case<FileTimeT, TimeT, TimeSpecT, Base, TK_32Bit, TK_128Bit>
    : public test_case<FileTimeT, TimeT, TimeSpecT, Base, TK_64Bit, TK_128Bit> {

};

template <class FileTimeT, class TimeT, class TimeSpecT, class Base>
struct test_case<FileTimeT, TimeT, TimeSpecT, Base, TK_64Bit, TK_64Bit>
    : public Base {

  using Base::convert_from_timespec;
  using Base::is_representable;
  using Base::max_nsec;
  using Base::max_seconds;
  using Base::min_nsec_timespec;
  using Base::min_seconds;

  static constexpr auto max_time_t = std::numeric_limits<TimeT>::max();
  static constexpr auto min_time_t = std::numeric_limits<TimeT>::min();

  static constexpr bool test_timespec() {
    static_assert(is_representable(TimeSpecT{max_seconds, max_nsec}), "");
    static_assert(!is_representable(TimeSpecT{max_seconds + 1, 0}), "");
    static_assert(!is_representable(TimeSpecT{max_seconds, max_nsec + 1}), "");
    static_assert(!is_representable(TimeSpecT{max_time_t, 0}), "");
    static_assert(is_representable(TimeSpecT{min_seconds, 0}), "");
    static_assert(
        is_representable(TimeSpecT{min_seconds - 1, min_nsec_timespec}), "");
    static_assert(
        is_representable(TimeSpecT{min_seconds - 1, min_nsec_timespec + 1}),
        "");
    static_assert(
        !is_representable(TimeSpecT{min_seconds - 1, min_nsec_timespec - 1}),
        "");
    static_assert(!is_representable(TimeSpecT{min_time_t, 999999999}), "");
    return true;
  }

  static constexpr bool test_file_time_type() {
    static_assert(Base::is_representable(FileTimeT::max()), "");
    static_assert(Base::is_representable(FileTimeT::min()), "");
    return true;
  }

  static constexpr bool test_convert_timespec() {
    static_assert(convert_from_timespec(TimeSpecT{max_seconds, max_nsec}) ==
                      FileTimeT::max(),
                  "");
    static_assert(convert_from_timespec(TimeSpecT{max_seconds, max_nsec - 1}) <
                      FileTimeT::max(),
                  "");
    static_assert(convert_from_timespec(TimeSpecT{max_seconds - 1, 999999999}) <
                      FileTimeT::max(),
                  "");
    static_assert(convert_from_timespec(TimeSpecT{
                      min_seconds - 1, min_nsec_timespec}) == FileTimeT::min(),
                  "");
    static_assert(convert_from_timespec(
                      TimeSpecT{min_seconds - 1, min_nsec_timespec + 1}) >
                      FileTimeT::min(),
                  "");
    static_assert(convert_from_timespec(TimeSpecT{min_seconds, 0}) >
                      FileTimeT::min(),
                  "");
    return true;
  }

  static bool test() {
    static_assert(test_timespec(), "");
    static_assert(test_file_time_type(), "");
    static_assert(test_convert_timespec(), "");
    return true;
  }
};

template <class FileTimeT, class TimeT, class TimeSpecT, class Base>
struct test_case<FileTimeT, TimeT, TimeSpecT, Base, TK_32Bit, TK_64Bit>
    : public Base {
  static constexpr auto max_time_t = std::numeric_limits<TimeT>::max();
  static constexpr auto min_time_t = std::numeric_limits<TimeT>::min();

  using Base::convert_from_timespec;
  using Base::is_representable;
  using Base::max_nsec;
  using Base::max_seconds;
  using Base::min_nsec_timespec;
  using Base::min_seconds;

  static constexpr bool test_timespec() {
    static_assert(is_representable(TimeSpecT{max_time_t, 999999999}), "");
    static_assert(is_representable(TimeSpecT{max_time_t, 1000000000}), "");
    static_assert(is_representable(TimeSpecT{min_time_t, 0}), "");
    return true;
  }

  static constexpr bool test_file_time_type() {
    static_assert(!is_representable(FileTimeT::max()), "");
    static_assert(!is_representable(FileTimeT::min()), "");
    static_assert(is_representable(FileTimeT(seconds(max_time_t))), "");
    static_assert(is_representable(FileTimeT(seconds(min_time_t))), "");
    return true;
  }

  static constexpr bool test_convert_timespec() {
    // FIXME add tests for 32 bit builds
    return true;
  }

  static bool test() {
    static_assert(test_timespec(), "");
    static_assert(test_file_time_type(), "");
    static_assert(test_convert_timespec(), "");
    return true;
  }
};

template <class FileTimeT, class TimeT, class TimeSpec, class Base,
          TestKind FileTimeTKind>
struct test_case<FileTimeT, TimeT, TimeSpec, Base, TK_FloatingPoint,
                 FileTimeTKind> : public Base {

  static bool test() { return true; }
};

template <class TimeT, class NSecT = long>
struct TestTimeSpec {
  TimeT tv_sec;
  NSecT tv_nsec;
};

template <class Dur>
struct TestClock {
  typedef Dur duration;
  typedef typename duration::rep rep;
  typedef typename duration::period period;
  typedef std::chrono::time_point<TestClock> time_point;
  static constexpr const bool is_steady = false;

  static time_point now() noexcept { return {}; }
};

template <class IntType, class Period = std::micro>
using TestFileTimeT = time_point<TestClock<duration<IntType, Period> > >;

int main() {
  { assert((test_case<file_time_type, time_t, struct timespec>::test())); }
  {
    assert((test_case<TestFileTimeT<int64_t>, int64_t,
                      TestTimeSpec<int64_t, long> >::test()));
  }
  {
    assert((test_case<TestFileTimeT<long long>, int32_t,
                      TestTimeSpec<int32_t, int32_t> >::test()));
  }
  {
    // Test that insane platforms like ppc64 linux, which use long double as time_t,
    // at least compile.
    assert((test_case<TestFileTimeT<long double>, double,
                      TestTimeSpec<long double, long> >::test()));
  }
#ifndef TEST_HAS_NO_INT128_T
  {
    assert((test_case<TestFileTimeT<__int128_t, std::nano>, int64_t,
                      TestTimeSpec<int64_t, long> >::test()));
  }
  {
    assert((test_case<TestFileTimeT<__int128_t, std::nano>, int32_t,
                      TestTimeSpec<int32_t, int32_t> >::test()));
  }
#endif
}
