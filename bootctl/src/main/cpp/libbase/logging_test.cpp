/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "android-base/logging.h"

#include <libgen.h>

#if defined(_WIN32)
#include <signal.h>
#endif

#include <memory>
#include <regex>
#include <string>
#include <thread>

#include "android-base/file.h"
#include "android-base/scopeguard.h"
#include "android-base/stringprintf.h"
#include "android-base/test_utils.h"

#include <gtest/gtest.h>

#ifdef __ANDROID__
#define HOST_TEST(suite, name) TEST(suite, DISABLED_ ## name)
#else
#define HOST_TEST(suite, name) TEST(suite, name)
#endif

#if defined(_WIN32)
static void ExitSignalAbortHandler(int) {
  _exit(3);
}
#endif

static void SuppressAbortUI() {
#if defined(_WIN32)
  // We really just want to call _set_abort_behavior(0, _CALL_REPORTFAULT) to
  // suppress the Windows Error Reporting dialog box, but that API is not
  // available in the OS-supplied C Runtime, msvcrt.dll, that we currently
  // use (it is available in the Visual Studio C runtime).
  //
  // Instead, we setup a SIGABRT handler, which is called in abort() right
  // before calling Windows Error Reporting. In the handler, we exit the
  // process just like abort() does.
  ASSERT_NE(SIG_ERR, signal(SIGABRT, ExitSignalAbortHandler));
#endif
}

TEST(logging, CHECK) {
  ASSERT_DEATH({SuppressAbortUI(); CHECK(false);}, "Check failed: false ");
  CHECK(true);

  ASSERT_DEATH({SuppressAbortUI(); CHECK_EQ(0, 1);}, "Check failed: 0 == 1 ");
  CHECK_EQ(0, 0);

  std::unique_ptr<int> p;
  ASSERT_DEATH(CHECK_NE(p, nullptr), "Check failed");
  CHECK_EQ(p, nullptr);
  CHECK_EQ(p, p);

  const char* kText = "Some text";
  ASSERT_DEATH(CHECK_NE(kText, kText), "Check failed");
  ASSERT_DEATH(CHECK_EQ(kText, nullptr), "Check failed.*null");
  CHECK_EQ(kText, kText);

  ASSERT_DEATH({SuppressAbortUI(); CHECK_STREQ("foo", "bar");},
               R"(Check failed: "foo" == "bar")");
  CHECK_STREQ("foo", "foo");

  // Test whether CHECK() and CHECK_STREQ() have a dangling if with no else.
  bool flag = false;
  if (true)
    CHECK(true);
  else
    flag = true;
  EXPECT_FALSE(flag) << "CHECK macro probably has a dangling if with no else";

  flag = false;
  if (true)
    CHECK_STREQ("foo", "foo");
  else
    flag = true;
  EXPECT_FALSE(flag) << "CHECK_STREQ probably has a dangling if with no else";
}

TEST(logging, DCHECK) {
  if (android::base::kEnableDChecks) {
    ASSERT_DEATH({SuppressAbortUI(); DCHECK(false);}, "DCheck failed: false ");
  }
  DCHECK(true);

  if (android::base::kEnableDChecks) {
    ASSERT_DEATH({SuppressAbortUI(); DCHECK_EQ(0, 1);}, "DCheck failed: 0 == 1 ");
  }
  DCHECK_EQ(0, 0);

  std::unique_ptr<int> p;
  if (android::base::kEnableDChecks) {
    ASSERT_DEATH(DCHECK_NE(p, nullptr), "DCheck failed");
  }
  DCHECK_EQ(p, nullptr);
  DCHECK_EQ(p, p);

  if (android::base::kEnableDChecks) {
    ASSERT_DEATH({SuppressAbortUI(); DCHECK_STREQ("foo", "bar");},
                 R"(DCheck failed: "foo" == "bar")");
  }
  DCHECK_STREQ("foo", "foo");

  // No testing whether we have a dangling else, possibly. That's inherent to the if (constexpr)
  // setup we intentionally chose to force type-checks of debug code even in release builds (so
  // we don't get more bit-rot).
}


#define CHECK_WOULD_LOG_DISABLED(severity)                                               \
  static_assert(android::base::severity < android::base::FATAL, "Bad input");            \
  for (size_t i = static_cast<size_t>(android::base::severity) + 1;                      \
       i <= static_cast<size_t>(android::base::FATAL);                                   \
       ++i) {                                                                            \
    {                                                                                    \
      android::base::ScopedLogSeverity sls2(static_cast<android::base::LogSeverity>(i)); \
      EXPECT_FALSE(WOULD_LOG(severity)) << i;                                            \
    }                                                                                    \
    {                                                                                    \
      android::base::ScopedLogSeverity sls2(static_cast<android::base::LogSeverity>(i)); \
      EXPECT_FALSE(WOULD_LOG(::android::base::severity)) << i;                           \
    }                                                                                    \
  }                                                                                      \

#define CHECK_WOULD_LOG_ENABLED(severity)                                                \
  for (size_t i = static_cast<size_t>(android::base::VERBOSE);                           \
       i <= static_cast<size_t>(android::base::severity);                                \
       ++i) {                                                                            \
    {                                                                                    \
      android::base::ScopedLogSeverity sls2(static_cast<android::base::LogSeverity>(i)); \
      EXPECT_TRUE(WOULD_LOG(severity)) << i;                                             \
    }                                                                                    \
    {                                                                                    \
      android::base::ScopedLogSeverity sls2(static_cast<android::base::LogSeverity>(i)); \
      EXPECT_TRUE(WOULD_LOG(::android::base::severity)) << i;                            \
    }                                                                                    \
  }                                                                                      \

TEST(logging, WOULD_LOG_FATAL) {
  CHECK_WOULD_LOG_ENABLED(FATAL);
}

TEST(logging, WOULD_LOG_FATAL_WITHOUT_ABORT_enabled) {
  CHECK_WOULD_LOG_ENABLED(FATAL_WITHOUT_ABORT);
}

TEST(logging, WOULD_LOG_ERROR_disabled) {
  CHECK_WOULD_LOG_DISABLED(ERROR);
}

TEST(logging, WOULD_LOG_ERROR_enabled) {
  CHECK_WOULD_LOG_ENABLED(ERROR);
}

TEST(logging, WOULD_LOG_WARNING_disabled) {
  CHECK_WOULD_LOG_DISABLED(WARNING);
}

TEST(logging, WOULD_LOG_WARNING_enabled) {
  CHECK_WOULD_LOG_ENABLED(WARNING);
}

TEST(logging, WOULD_LOG_INFO_disabled) {
  CHECK_WOULD_LOG_DISABLED(INFO);
}

TEST(logging, WOULD_LOG_INFO_enabled) {
  CHECK_WOULD_LOG_ENABLED(INFO);
}

TEST(logging, WOULD_LOG_DEBUG_disabled) {
  CHECK_WOULD_LOG_DISABLED(DEBUG);
}

TEST(logging, WOULD_LOG_DEBUG_enabled) {
  CHECK_WOULD_LOG_ENABLED(DEBUG);
}

TEST(logging, WOULD_LOG_VERBOSE_disabled) {
  CHECK_WOULD_LOG_DISABLED(VERBOSE);
}

TEST(logging, WOULD_LOG_VERBOSE_enabled) {
  CHECK_WOULD_LOG_ENABLED(VERBOSE);
}

#undef CHECK_WOULD_LOG_DISABLED
#undef CHECK_WOULD_LOG_ENABLED


#if !defined(_WIN32)
static std::string make_log_pattern(android::base::LogSeverity severity,
                                    const char* message) {
  static const char log_characters[] = "VDIWEFF";
  static_assert(arraysize(log_characters) - 1 == android::base::FATAL + 1,
                "Mismatch in size of log_characters and values in LogSeverity");
  char log_char = log_characters[severity];
  std::string holder(__FILE__);
  return android::base::StringPrintf(
      "%c \\d+-\\d+ \\d+:\\d+:\\d+ \\s*\\d+ \\s*\\d+ %s:\\d+] %s",
      log_char, basename(&holder[0]), message);
}
#endif

static void CheckMessage(const std::string& output, android::base::LogSeverity severity,
                         const char* expected, const char* expected_tag = nullptr) {
  // We can't usefully check the output of any of these on Windows because we
  // don't have std::regex, but we can at least make sure we printed at least as
  // many characters are in the log message.
  ASSERT_GT(output.length(), strlen(expected));
  ASSERT_NE(nullptr, strstr(output.c_str(), expected)) << output;
  if (expected_tag != nullptr) {
    ASSERT_NE(nullptr, strstr(output.c_str(), expected_tag)) << output;
  }

#if !defined(_WIN32)
  std::string regex_str;
  if (expected_tag != nullptr) {
    regex_str.append(expected_tag);
    regex_str.append(" ");
  }
  regex_str.append(make_log_pattern(severity, expected));
  std::regex message_regex(regex_str);
  ASSERT_TRUE(std::regex_search(output, message_regex)) << output;
#endif
}

static void CheckMessage(CapturedStderr& cap, android::base::LogSeverity severity,
                         const char* expected, const char* expected_tag = nullptr) {
  cap.Stop();
  std::string output = cap.str();
  return CheckMessage(output, severity, expected, expected_tag);
}

#define CHECK_LOG_STREAM_DISABLED(severity)                      \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    LOG_STREAM(severity) << "foo bar";                           \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }                                                              \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    LOG_STREAM(::android::base::severity) << "foo bar";          \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }

#define CHECK_LOG_STREAM_ENABLED(severity) \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    LOG_STREAM(severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar"); \
  } \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    LOG_STREAM(::android::base::severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar"); \
  } \

TEST(logging, LOG_STREAM_FATAL_WITHOUT_ABORT_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(FATAL_WITHOUT_ABORT));
}

TEST(logging, LOG_STREAM_ERROR_disabled) {
  CHECK_LOG_STREAM_DISABLED(ERROR);
}

TEST(logging, LOG_STREAM_ERROR_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(ERROR));
}

TEST(logging, LOG_STREAM_WARNING_disabled) {
  CHECK_LOG_STREAM_DISABLED(WARNING);
}

TEST(logging, LOG_STREAM_WARNING_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(WARNING));
}

TEST(logging, LOG_STREAM_INFO_disabled) {
  CHECK_LOG_STREAM_DISABLED(INFO);
}

TEST(logging, LOG_STREAM_INFO_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(INFO));
}

TEST(logging, LOG_STREAM_DEBUG_disabled) {
  CHECK_LOG_STREAM_DISABLED(DEBUG);
}

TEST(logging, LOG_STREAM_DEBUG_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(DEBUG));
}

TEST(logging, LOG_STREAM_VERBOSE_disabled) {
  CHECK_LOG_STREAM_DISABLED(VERBOSE);
}

TEST(logging, LOG_STREAM_VERBOSE_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_STREAM_ENABLED(VERBOSE));
}

#undef CHECK_LOG_STREAM_DISABLED
#undef CHECK_LOG_STREAM_ENABLED

#define CHECK_LOG_DISABLED(severity)                             \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    LOG(severity) << "foo bar";                                  \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }                                                              \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    LOG(::android::base::severity) << "foo bar";                 \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }

#define CHECK_LOG_ENABLED(severity) \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    LOG(severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar"); \
  } \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    LOG(::android::base::severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar"); \
  } \

TEST(logging, LOG_FATAL) {
  ASSERT_DEATH({SuppressAbortUI(); LOG(FATAL) << "foobar";}, "foobar");
  ASSERT_DEATH({SuppressAbortUI(); LOG(::android::base::FATAL) << "foobar";}, "foobar");
}

TEST(logging, LOG_FATAL_WITHOUT_ABORT_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(FATAL_WITHOUT_ABORT));
}

TEST(logging, LOG_ERROR_disabled) {
  CHECK_LOG_DISABLED(ERROR);
}

TEST(logging, LOG_ERROR_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(ERROR));
}

TEST(logging, LOG_WARNING_disabled) {
  CHECK_LOG_DISABLED(WARNING);
}

TEST(logging, LOG_WARNING_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(WARNING));
}

TEST(logging, LOG_INFO_disabled) {
  CHECK_LOG_DISABLED(INFO);
}

TEST(logging, LOG_INFO_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(INFO));
}

TEST(logging, LOG_DEBUG_disabled) {
  CHECK_LOG_DISABLED(DEBUG);
}

TEST(logging, LOG_DEBUG_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(DEBUG));
}

TEST(logging, LOG_VERBOSE_disabled) {
  CHECK_LOG_DISABLED(VERBOSE);
}

TEST(logging, LOG_VERBOSE_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_LOG_ENABLED(VERBOSE));
}

#undef CHECK_LOG_DISABLED
#undef CHECK_LOG_ENABLED

TEST(logging, LOG_complex_param) {
#define CHECK_LOG_COMBINATION(use_scoped_log_severity_info, use_logging_severity_info)         \
  {                                                                                            \
    android::base::ScopedLogSeverity sls(                                                      \
        (use_scoped_log_severity_info) ? ::android::base::INFO : ::android::base::WARNING);    \
    CapturedStderr cap;                                                                        \
    LOG((use_logging_severity_info) ? ::android::base::INFO : ::android::base::WARNING)        \
        << "foobar";                                                                           \
    if ((use_scoped_log_severity_info) || !(use_logging_severity_info)) {                      \
      ASSERT_NO_FATAL_FAILURE(CheckMessage(                                                    \
          cap, (use_logging_severity_info) ? ::android::base::INFO : ::android::base::WARNING, \
          "foobar"));                                                                          \
    } else {                                                                                   \
      cap.Stop();                                                                              \
      ASSERT_EQ("", cap.str());                                                                \
    }                                                                                          \
  }

  CHECK_LOG_COMBINATION(false,false);
  CHECK_LOG_COMBINATION(false,true);
  CHECK_LOG_COMBINATION(true,false);
  CHECK_LOG_COMBINATION(true,true);

#undef CHECK_LOG_COMBINATION
}


TEST(logging, LOG_does_not_clobber_errno) {
  CapturedStderr cap;
  errno = 12345;
  LOG(INFO) << (errno = 67890);
  EXPECT_EQ(12345, errno) << "errno was not restored";

  ASSERT_NO_FATAL_FAILURE(CheckMessage(cap, android::base::INFO, "67890"));
}

TEST(logging, PLOG_does_not_clobber_errno) {
  CapturedStderr cap;
  errno = 12345;
  PLOG(INFO) << (errno = 67890);
  EXPECT_EQ(12345, errno) << "errno was not restored";

  ASSERT_NO_FATAL_FAILURE(CheckMessage(cap, android::base::INFO, "67890"));
}

TEST(logging, LOG_does_not_have_dangling_if) {
  CapturedStderr cap; // So the logging below has no side-effects.

  // Do the test two ways: once where we hypothesize that LOG()'s if
  // will evaluate to true (when severity is high enough) and once when we
  // expect it to evaluate to false (when severity is not high enough).
  bool flag = false;
  if (true)
    LOG(INFO) << "foobar";
  else
    flag = true;

  EXPECT_FALSE(flag) << "LOG macro probably has a dangling if with no else";

  flag = false;
  if (true)
    LOG(VERBOSE) << "foobar";
  else
    flag = true;

  EXPECT_FALSE(flag) << "LOG macro probably has a dangling if with no else";
}

#define CHECK_PLOG_DISABLED(severity)                            \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    PLOG(severity) << "foo bar";                                 \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }                                                              \
  {                                                              \
    android::base::ScopedLogSeverity sls1(android::base::FATAL); \
    CapturedStderr cap1;                                         \
    PLOG(severity) << "foo bar";                                 \
    cap1.Stop();                                                 \
    ASSERT_EQ("", cap1.str());                                   \
  }

#define CHECK_PLOG_ENABLED(severity) \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    errno = ENOENT; \
    PLOG(severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar: No such file or directory"); \
  } \
  { \
    android::base::ScopedLogSeverity sls2(android::base::severity); \
    CapturedStderr cap2; \
    errno = ENOENT; \
    PLOG(severity) << "foobar"; \
    CheckMessage(cap2, android::base::severity, "foobar: No such file or directory"); \
  } \

TEST(logging, PLOG_FATAL) {
  ASSERT_DEATH({SuppressAbortUI(); PLOG(FATAL) << "foobar";}, "foobar");
  ASSERT_DEATH({SuppressAbortUI(); PLOG(::android::base::FATAL) << "foobar";}, "foobar");
}

TEST(logging, PLOG_FATAL_WITHOUT_ABORT_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(FATAL_WITHOUT_ABORT));
}

TEST(logging, PLOG_ERROR_disabled) {
  CHECK_PLOG_DISABLED(ERROR);
}

TEST(logging, PLOG_ERROR_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(ERROR));
}

TEST(logging, PLOG_WARNING_disabled) {
  CHECK_PLOG_DISABLED(WARNING);
}

TEST(logging, PLOG_WARNING_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(WARNING));
}

TEST(logging, PLOG_INFO_disabled) {
  CHECK_PLOG_DISABLED(INFO);
}

TEST(logging, PLOG_INFO_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(INFO));
}

TEST(logging, PLOG_DEBUG_disabled) {
  CHECK_PLOG_DISABLED(DEBUG);
}

TEST(logging, PLOG_DEBUG_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(DEBUG));
}

TEST(logging, PLOG_VERBOSE_disabled) {
  CHECK_PLOG_DISABLED(VERBOSE);
}

TEST(logging, PLOG_VERBOSE_enabled) {
  ASSERT_NO_FATAL_FAILURE(CHECK_PLOG_ENABLED(VERBOSE));
}

#undef CHECK_PLOG_DISABLED
#undef CHECK_PLOG_ENABLED


TEST(logging, UNIMPLEMENTED) {
  std::string expected = android::base::StringPrintf("%s unimplemented ", __PRETTY_FUNCTION__);

  CapturedStderr cap;
  errno = ENOENT;
  UNIMPLEMENTED(ERROR);
  ASSERT_NO_FATAL_FAILURE(CheckMessage(cap, android::base::ERROR, expected.c_str()));
}

static void NoopAborter(const char* msg ATTRIBUTE_UNUSED) {
  LOG(ERROR) << "called noop";
}

TEST(logging, LOG_FATAL_NOOP_ABORTER) {
  CapturedStderr cap;
  {
    android::base::SetAborter(NoopAborter);

    android::base::ScopedLogSeverity sls(android::base::ERROR);
    LOG(FATAL) << "foobar";
    cap.Stop();

    android::base::SetAborter(android::base::DefaultAborter);
  }
  std::string output = cap.str();
  ASSERT_NO_FATAL_FAILURE(CheckMessage(output, android::base::FATAL, "foobar"));
  ASSERT_NO_FATAL_FAILURE(CheckMessage(output, android::base::ERROR, "called noop"));

  ASSERT_DEATH({SuppressAbortUI(); LOG(FATAL) << "foobar";}, "foobar");
}

struct CountLineAborter {
  static void CountLineAborterFunction(const char* msg) {
    while (*msg != 0) {
      if (*msg == '\n') {
        newline_count++;
      }
      msg++;
    }
  }
  static size_t newline_count;
};
size_t CountLineAborter::newline_count = 0;

TEST(logging, LOG_FATAL_ABORTER_MESSAGE) {
  CountLineAborter::newline_count = 0;
  android::base::SetAborter(CountLineAborter::CountLineAborterFunction);

  android::base::ScopedLogSeverity sls(android::base::ERROR);
  CapturedStderr cap;
  LOG(FATAL) << "foo\nbar";

  EXPECT_EQ(CountLineAborter::newline_count, 1U);
}

TEST(logging, SetAborterReturnsOldFunction) {
  // std::function is not comparable, it only supports a null check.
  android::base::AbortFunction old_aborter;
  EXPECT_FALSE(old_aborter);
  old_aborter = android::base::SetAborter(android::base::DefaultAborter);
  EXPECT_TRUE(old_aborter);
}

__attribute__((constructor)) void TestLoggingInConstructor() {
  LOG(ERROR) << "foobar";
}

TEST(logging, StdioLogger) {
  CapturedStderr cap_err;
  CapturedStdout cap_out;
  android::base::SetLogger(android::base::StdioLogger);
  LOG(INFO) << "out";
  LOG(ERROR) << "err";
  cap_err.Stop();
  cap_out.Stop();

  // For INFO we expect just the literal "out\n".
  ASSERT_EQ("out\n", cap_out.str());
  // Whereas ERROR logging includes the program name.
  ASSERT_EQ(android::base::Basename(android::base::GetExecutablePath()) + ": err\n", cap_err.str());
}

TEST(logging, SetLoggerReturnsOldFunction) {
  // std::function is not comparable, it only supports a null check.
  android::base::LogFunction old_function;
  EXPECT_FALSE(old_function);
  old_function = android::base::SetLogger(android::base::StdioLogger);
  EXPECT_TRUE(old_function);
}

TEST(logging, ForkSafe) {
#if !defined(_WIN32)
  using namespace android::base;
  SetLogger(
      [&](LogId, LogSeverity, const char*, const char*, unsigned int, const char*) { sleep(3); });

  auto guard = make_scope_guard([&] {
#ifdef __ANDROID__
    SetLogger(LogdLogger());
#else
    SetLogger(StderrLogger);
#endif
  });

  auto thread = std::thread([] {
    LOG(ERROR) << "This should sleep for 3 seconds, long enough to fork another process, if there "
                  "is no intervention";
  });
  thread.detach();

  auto pid = fork();
  ASSERT_NE(-1, pid);

  if (pid == 0) {
    // Reset the logger, so the next message doesn't sleep().
    SetLogger([](LogId, LogSeverity, const char*, const char*, unsigned int, const char*) {});
    LOG(ERROR) << "This should succeed in the child, only if libbase is forksafe.";
    _exit(EXIT_SUCCESS);
  }

  // Wait for up to 3 seconds for the child to exit.
  int tries = 3;
  bool found_child = false;
  while (tries-- > 0) {
    auto result = waitpid(pid, nullptr, WNOHANG);
    EXPECT_NE(-1, result);
    if (result == pid) {
      found_child = true;
      break;
    }
    sleep(1);
  }

  EXPECT_TRUE(found_child);

  // Kill the child if it did not exit.
  if (!found_child) {
    kill(pid, SIGKILL);
  }
#endif
}
