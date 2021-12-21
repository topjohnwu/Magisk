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

// file_time_type last_write_time(const path& p);
// file_time_type last_write_time(const path& p, std::error_code& ec) noexcept;
// void last_write_time(const path& p, file_time_type new_time);
// void last_write_time(const path& p, file_time_type new_type,
//                      std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <chrono>
#include <fstream>
#include <cstdlib>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

#include <sys/stat.h>
#include <iostream>

#include <fcntl.h>
#include <sys/time.h>

using namespace fs;

using TimeSpec = struct ::timespec;
using StatT = struct ::stat;

using Sec = std::chrono::duration<file_time_type::rep>;
using Hours = std::chrono::hours;
using Minutes = std::chrono::minutes;
using MicroSec = std::chrono::duration<file_time_type::rep, std::micro>;
using NanoSec = std::chrono::duration<file_time_type::rep, std::nano>;
using std::chrono::duration_cast;

#if defined(__APPLE__)
TimeSpec extract_mtime(StatT const& st) { return st.st_mtimespec; }
TimeSpec extract_atime(StatT const& st) { return st.st_atimespec; }
#else
TimeSpec extract_mtime(StatT const& st) { return st.st_mtim; }
TimeSpec extract_atime(StatT const& st) { return st.st_atim; }
#endif

bool ConvertToTimeSpec(TimeSpec& ts, file_time_type ft) {
  using SecFieldT = decltype(TimeSpec::tv_sec);
  using NSecFieldT = decltype(TimeSpec::tv_nsec);
  using SecLim = std::numeric_limits<SecFieldT>;
  using NSecLim = std::numeric_limits<NSecFieldT>;

  auto secs = duration_cast<Sec>(ft.time_since_epoch());
  auto nsecs = duration_cast<NanoSec>(ft.time_since_epoch() - secs);
  if (nsecs.count() < 0) {
    if (Sec::min().count() > SecLim::min()) {
      secs += Sec(1);
      nsecs -= Sec(1);
    } else {
      nsecs = NanoSec(0);
    }
  }
  if (SecLim::max() < secs.count() || SecLim::min() > secs.count())
    return false;
  if (NSecLim::max() < nsecs.count() || NSecLim::min() > nsecs.count())
    return false;
  ts.tv_sec = secs.count();
  ts.tv_nsec = nsecs.count();
  return true;
}

bool ConvertFromTimeSpec(file_time_type& ft, TimeSpec ts) {
  auto secs_part = duration_cast<file_time_type::duration>(Sec(ts.tv_sec));
  if (duration_cast<Sec>(secs_part).count() != ts.tv_sec)
    return false;
  auto subsecs = duration_cast<file_time_type::duration>(NanoSec(ts.tv_nsec));
  auto dur = secs_part + subsecs;
  if (dur < secs_part && subsecs.count() >= 0)
    return false;
  ft = file_time_type(dur);
  return true;
}

bool CompareTimeExact(TimeSpec ts, TimeSpec ts2) {
  return ts2.tv_sec == ts.tv_sec && ts2.tv_nsec == ts.tv_nsec;
}
bool CompareTimeExact(file_time_type ft, TimeSpec ts) {
  TimeSpec ts2 = {};
  if (!ConvertToTimeSpec(ts2, ft))
    return false;
  return CompareTimeExact(ts, ts2);
}
bool CompareTimeExact(TimeSpec ts, file_time_type ft) {
  return CompareTimeExact(ft, ts);
}

struct Times {
  TimeSpec access, write;
};

Times GetTimes(path const& p) {
    StatT st;
    if (::stat(p.c_str(), &st) == -1) {
        std::error_code ec(errno, std::generic_category());
#ifndef TEST_HAS_NO_EXCEPTIONS
        throw ec;
#else
        std::cerr << ec.message() << std::endl;
        std::exit(EXIT_FAILURE);
#endif
    }
    return {extract_atime(st), extract_mtime(st)};
}

TimeSpec LastAccessTime(path const& p) { return GetTimes(p).access; }

TimeSpec LastWriteTime(path const& p) { return GetTimes(p).write; }

Times GetSymlinkTimes(path const& p) {
  StatT st;
  if (::lstat(p.c_str(), &st) == -1) {
    std::error_code ec(errno, std::generic_category());
#ifndef TEST_HAS_NO_EXCEPTIONS
        throw ec;
#else
        std::cerr << ec.message() << std::endl;
        std::exit(EXIT_FAILURE);
#endif
    }
    Times res;
    res.access = extract_atime(st);
    res.write = extract_mtime(st);
    return res;
}

namespace {

// In some configurations, the comparison is tautological and the test is valid.
// We disable the warning so that we can actually test it regardless. Also, that
// diagnostic is pretty new, so also don't fail if old clang does not support it
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wtautological-constant-compare"
#endif

static const bool SupportsNegativeTimes = [] {
  using namespace std::chrono;
  std::error_code ec;
  TimeSpec old_write_time, new_write_time;
  { // WARNING: Do not assert in this scope.
    scoped_test_env env;
    const path file = env.create_file("file", 42);
    old_write_time = LastWriteTime(file);
    file_time_type tp(seconds(-5));
    fs::last_write_time(file, tp, ec);
    new_write_time = LastWriteTime(file);
  }

  return !ec && new_write_time.tv_sec < 0;
}();

static const bool SupportsMaxTime = [] {
  using namespace std::chrono;
  TimeSpec max_ts = {};
  if (!ConvertToTimeSpec(max_ts, file_time_type::max()))
    return false;

  std::error_code ec;
  TimeSpec old_write_time, new_write_time;
  { // WARNING: Do not assert in this scope.
    scoped_test_env env;
    const path file = env.create_file("file", 42);
    old_write_time = LastWriteTime(file);
    file_time_type tp = file_time_type::max();
    fs::last_write_time(file, tp, ec);
    new_write_time = LastWriteTime(file);
  }
  return !ec && new_write_time.tv_sec > max_ts.tv_sec - 1;
}();

static const bool SupportsMinTime = [] {
  using namespace std::chrono;
  TimeSpec min_ts = {};
  if (!ConvertToTimeSpec(min_ts, file_time_type::min()))
    return false;
  std::error_code ec;
  TimeSpec old_write_time, new_write_time;
  { // WARNING: Do not assert in this scope.
    scoped_test_env env;
    const path file = env.create_file("file", 42);
    old_write_time = LastWriteTime(file);
    file_time_type tp = file_time_type::min();
    fs::last_write_time(file, tp, ec);
    new_write_time = LastWriteTime(file);
  }
  return !ec && new_write_time.tv_sec < min_ts.tv_sec + 1;
}();

static const bool SupportsNanosecondRoundTrip = [] {
  NanoSec ns(3);
  static_assert(std::is_same<file_time_type::period, std::nano>::value, "");

  // Test that the system call we use to set the times also supports nanosecond
  // resolution. (utimes does not)
  file_time_type ft(ns);
  {
    scoped_test_env env;
    const path p = env.create_file("file", 42);
    last_write_time(p, ft);
    return last_write_time(p) == ft;
  }
}();

// The HFS+ filesystem (used by default before macOS 10.13) stores timestamps at
// a 1-second granularity, and APFS (now the default) at a 1 nanosecond granularity.
// 1-second granularity is also the norm on many of the supported filesystems
// on Linux as well.
static const bool WorkaroundStatTruncatesToSeconds = [] {
  MicroSec micros(3);
  static_assert(std::is_same<file_time_type::period, std::nano>::value, "");

  file_time_type ft(micros);
  {
    scoped_test_env env;
    const path p = env.create_file("file", 42);
    if (LastWriteTime(p).tv_nsec != 0)
      return false;
    last_write_time(p, ft);
    return last_write_time(p) != ft && LastWriteTime(p).tv_nsec == 0;
  }
}();

static const bool SupportsMinRoundTrip = [] {
  TimeSpec ts = {};
  if (!ConvertToTimeSpec(ts, file_time_type::min()))
    return false;
  file_time_type min_val = {};
  if (!ConvertFromTimeSpec(min_val, ts))
    return false;
  return min_val == file_time_type::min();
}();

} // end namespace

static bool CompareTime(TimeSpec t1, TimeSpec t2) {
  if (SupportsNanosecondRoundTrip)
    return CompareTimeExact(t1, t2);
  if (t1.tv_sec != t2.tv_sec)
    return false;

  auto diff = std::abs(t1.tv_nsec - t2.tv_nsec);
  if (WorkaroundStatTruncatesToSeconds)
   return diff < duration_cast<NanoSec>(Sec(1)).count();
  return diff < duration_cast<NanoSec>(MicroSec(1)).count();
}

static bool CompareTime(file_time_type t1, TimeSpec t2) {
  TimeSpec ts1 = {};
  if (!ConvertToTimeSpec(ts1, t1))
    return false;
  return CompareTime(ts1, t2);
}

static bool CompareTime(TimeSpec t1, file_time_type t2) {
  return CompareTime(t2, t1);
}

static bool CompareTime(file_time_type t1, file_time_type t2) {
  auto min_secs = duration_cast<Sec>(file_time_type::min().time_since_epoch());
  bool IsMin =
      t1.time_since_epoch() < min_secs || t2.time_since_epoch() < min_secs;

  if (SupportsNanosecondRoundTrip && (!IsMin || SupportsMinRoundTrip))
    return t1 == t2;
  if (IsMin) {
    return duration_cast<Sec>(t1.time_since_epoch()) ==
           duration_cast<Sec>(t2.time_since_epoch());
  }
  file_time_type::duration dur;
  if (t1 > t2)
    dur = t1 - t2;
  else
    dur = t2 - t1;
  if (WorkaroundStatTruncatesToSeconds)
    return duration_cast<Sec>(dur).count() == 0;
  return duration_cast<MicroSec>(dur).count() == 0;
}

// Check if a time point is representable on a given filesystem. Check that:
// (A) 'tp' is representable as a time_t
// (B) 'tp' is non-negative or the filesystem supports negative times.
// (C) 'tp' is not 'file_time_type::max()' or the filesystem supports the max
//     value.
// (D) 'tp' is not 'file_time_type::min()' or the filesystem supports the min
//     value.
inline bool TimeIsRepresentableByFilesystem(file_time_type tp) {
  TimeSpec ts = {};
  if (!ConvertToTimeSpec(ts, tp))
    return false;
  else if (tp.time_since_epoch().count() < 0 && !SupportsNegativeTimes)
    return false;
  else if (tp == file_time_type::max() && !SupportsMaxTime)
    return false;
  else if (tp == file_time_type::min() && !SupportsMinTime)
    return false;
  return true;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

// Create a sub-second duration using the smallest period the filesystem supports.
file_time_type::duration SubSec(long long val) {
  using SubSecT = file_time_type::duration;
  if (SupportsNanosecondRoundTrip) {
    return duration_cast<SubSecT>(NanoSec(val));
  } else {
    return duration_cast<SubSecT>(MicroSec(val));
  }
}

TEST_SUITE(last_write_time_test_suite)

TEST_CASE(signature_test)
{
    const file_time_type t;
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(last_write_time(p)), file_time_type);
    ASSERT_SAME_TYPE(decltype(last_write_time(p, ec)), file_time_type);
    ASSERT_SAME_TYPE(decltype(last_write_time(p, t)), void);
    ASSERT_SAME_TYPE(decltype(last_write_time(p, t, ec)), void);
    ASSERT_NOT_NOEXCEPT(last_write_time(p));
    ASSERT_NOT_NOEXCEPT(last_write_time(p, t));
    ASSERT_NOEXCEPT(last_write_time(p, ec));
    ASSERT_NOEXCEPT(last_write_time(p, t, ec));
}

TEST_CASE(read_last_write_time_static_env_test)
{
    using C = file_time_type::clock;
    file_time_type min = file_time_type::min();
    {
        file_time_type ret = last_write_time(StaticEnv::File);
        TEST_CHECK(ret != min);
        TEST_CHECK(ret < C::now());
        TEST_CHECK(CompareTime(ret, LastWriteTime(StaticEnv::File)));

        file_time_type ret2 = last_write_time(StaticEnv::SymlinkToFile);
        TEST_CHECK(CompareTime(ret, ret2));
        TEST_CHECK(CompareTime(ret2, LastWriteTime(StaticEnv::SymlinkToFile)));
    }
    {
        file_time_type ret = last_write_time(StaticEnv::Dir);
        TEST_CHECK(ret != min);
        TEST_CHECK(ret < C::now());
        TEST_CHECK(CompareTime(ret, LastWriteTime(StaticEnv::Dir)));

        file_time_type ret2 = last_write_time(StaticEnv::SymlinkToDir);
        TEST_CHECK(CompareTime(ret, ret2));
        TEST_CHECK(CompareTime(ret2, LastWriteTime(StaticEnv::SymlinkToDir)));
    }
}

TEST_CASE(get_last_write_time_dynamic_env_test)
{
    using Clock = file_time_type::clock;
    using Sec = std::chrono::seconds;
    scoped_test_env env;

    const path file = env.create_file("file", 42);
    const path dir = env.create_dir("dir");

    const auto file_times = GetTimes(file);
    const TimeSpec file_write_time = file_times.write;
    const auto dir_times = GetTimes(dir);
    const TimeSpec dir_write_time = dir_times.write;

    file_time_type ftime = last_write_time(file);
    TEST_CHECK(Clock::to_time_t(ftime) == file_write_time.tv_sec);
    TEST_CHECK(CompareTime(ftime, file_write_time));

    file_time_type dtime = last_write_time(dir);
    TEST_CHECK(Clock::to_time_t(dtime) == dir_write_time.tv_sec);
    TEST_CHECK(CompareTime(dtime, dir_write_time));

    SleepFor(Sec(2));

    // update file and add a file to the directory. Make sure the times increase.
    std::ofstream of(file, std::ofstream::app);
    of << "hello";
    of.close();
    env.create_file("dir/file1", 1);

    file_time_type ftime2 = last_write_time(file);
    file_time_type dtime2 = last_write_time(dir);

    TEST_CHECK(ftime2 > ftime);
    TEST_CHECK(dtime2 > dtime);
    TEST_CHECK(CompareTime(LastWriteTime(file), ftime2));
    TEST_CHECK(CompareTime(LastWriteTime(dir), dtime2));
}


TEST_CASE(set_last_write_time_dynamic_env_test)
{
    using Clock = file_time_type::clock;
    scoped_test_env env;

    const path file = env.create_file("file", 42);
    const path dir = env.create_dir("dir");
    const auto now = Clock::now();
    const file_time_type epoch_time = now - now.time_since_epoch();

    const file_time_type future_time = now + Hours(3) + Sec(42) + SubSec(17);
    const file_time_type past_time = now - Minutes(3) - Sec(42) - SubSec(17);
    const file_time_type before_epoch_time =
        epoch_time - Minutes(3) - Sec(42) - SubSec(17);
    // FreeBSD has a bug in their utimes implementation where the time is not update
    // when the number of seconds is '-1'.
#if defined(__FreeBSD__) || defined(__NetBSD__)
    const file_time_type just_before_epoch_time =
        epoch_time - Sec(2) - SubSec(17);
#else
    const file_time_type just_before_epoch_time = epoch_time - SubSec(17);
#endif

    struct TestCase {
      const char * case_name;
      path p;
      file_time_type new_time;
    } cases[] = {
        {"file, epoch_time", file, epoch_time},
        {"dir, epoch_time", dir, epoch_time},
        {"file, future_time", file, future_time},
        {"dir, future_time", dir, future_time},
        {"file, past_time", file, past_time},
        {"dir, past_time", dir, past_time},
        {"file, before_epoch_time", file, before_epoch_time},
        {"dir, before_epoch_time", dir, before_epoch_time},
        {"file, just_before_epoch_time", file, just_before_epoch_time},
        {"dir, just_before_epoch_time", dir, just_before_epoch_time}
    };
    for (const auto& TC : cases) {
        std::cerr << "Test Case = " << TC.case_name << "\n";
        const auto old_times = GetTimes(TC.p);
        file_time_type old_time;
        TEST_REQUIRE(ConvertFromTimeSpec(old_time, old_times.write));

        std::error_code ec = GetTestEC();
        last_write_time(TC.p, TC.new_time, ec);
        TEST_CHECK(!ec);

        ec = GetTestEC();
        file_time_type  got_time = last_write_time(TC.p, ec);
        TEST_REQUIRE(!ec);

        if (TimeIsRepresentableByFilesystem(TC.new_time)) {
            TEST_CHECK(got_time != old_time);
            TEST_CHECK(CompareTime(got_time, TC.new_time));
            TEST_CHECK(CompareTime(LastAccessTime(TC.p), old_times.access));
        }
    }
}

TEST_CASE(last_write_time_symlink_test)
{
    using Clock = file_time_type::clock;

    scoped_test_env env;

    const path file = env.create_file("file", 42);
    const path sym = env.create_symlink("file", "sym");

    const file_time_type new_time = Clock::now() + Hours(3);

    const auto old_times = GetTimes(sym);
    const auto old_sym_times = GetSymlinkTimes(sym);

    std::error_code ec = GetTestEC();
    last_write_time(sym, new_time, ec);
    TEST_CHECK(!ec);

    file_time_type  got_time = last_write_time(sym);
    TEST_CHECK(!CompareTime(got_time, old_times.write));
    if (!WorkaroundStatTruncatesToSeconds) {
      TEST_CHECK(got_time == new_time);
    } else {
      TEST_CHECK(CompareTime(got_time, new_time));
    }

    TEST_CHECK(CompareTime(LastWriteTime(file), new_time));
    TEST_CHECK(CompareTime(LastAccessTime(sym), old_times.access));
    Times sym_times = GetSymlinkTimes(sym);
    TEST_CHECK(CompareTime(sym_times.write, old_sym_times.write));
}


TEST_CASE(test_write_min_time)
{
    scoped_test_env env;
    const path p = env.create_file("file", 42);
    const file_time_type old_time = last_write_time(p);
    file_time_type new_time = file_time_type::min();

    std::error_code ec = GetTestEC();
    last_write_time(p, new_time, ec);
    file_time_type tt = last_write_time(p);

    if (TimeIsRepresentableByFilesystem(new_time)) {
        TEST_CHECK(!ec);
        TEST_CHECK(CompareTime(tt, new_time));

        last_write_time(p, old_time);
        new_time = file_time_type::min() + SubSec(1);

        ec = GetTestEC();
        last_write_time(p, new_time, ec);
        tt = last_write_time(p);

        if (TimeIsRepresentableByFilesystem(new_time)) {
            TEST_CHECK(!ec);
            TEST_CHECK(CompareTime(tt, new_time));
        } else {
          TEST_CHECK(ErrorIs(ec, std::errc::value_too_large));
          TEST_CHECK(tt == old_time);
        }
    } else {
      TEST_CHECK(ErrorIs(ec, std::errc::value_too_large));
      TEST_CHECK(tt == old_time);
    }
}

TEST_CASE(test_write_max_time) {
  scoped_test_env env;
  const path p = env.create_file("file", 42);
  const file_time_type old_time = last_write_time(p);
  file_time_type new_time = file_time_type::max();

  std::error_code ec = GetTestEC();
  last_write_time(p, new_time, ec);
  file_time_type tt = last_write_time(p);

  if (TimeIsRepresentableByFilesystem(new_time)) {
    TEST_CHECK(!ec);
    TEST_CHECK(CompareTime(tt, new_time));
  } else {
    TEST_CHECK(ErrorIs(ec, std::errc::value_too_large));
    TEST_CHECK(tt == old_time);
  }
}

TEST_CASE(test_value_on_failure)
{
    const path p = StaticEnv::DNE;
    std::error_code ec = GetTestEC();
    TEST_CHECK(last_write_time(p, ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));
}

TEST_CASE(test_exists_fails)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path file = env.create_file("dir/file", 42);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    TEST_CHECK(last_write_time(file, ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(file, std::errc::permission_denied,
                             "last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, last_write_time(file));
}

TEST_SUITE_END()
