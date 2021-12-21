/*
 * Copyright (C) 2013-2016 The Android Open Source Project
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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>

#include <android-base/file.h>
#include <android-base/macros.h>
#include <android-base/scopeguard.h>
#include <android-base/stringprintf.h>
#ifdef __ANDROID__  // includes sys/properties.h which does not exist outside
#include <cutils/properties.h>
#endif
#include <gtest/gtest.h>
#include <log/log_event_list.h>
#include <log/log_properties.h>
#include <log/log_read.h>
#include <log/logprint.h>
#include <private/android_filesystem_config.h>
#include <private/android_logger.h>

using android::base::make_scope_guard;

// #define ENABLE_FLAKY_TESTS

// enhanced version of LOG_FAILURE_RETRY to add support for EAGAIN and
// non-syscall libs. Since we are only using this in the emergency of
// a signal to stuff a terminating code into the logs, we will spin rather
// than try a usleep.
#define LOG_FAILURE_RETRY(exp)                                           \
  ({                                                                     \
    typeof(exp) _rc;                                                     \
    do {                                                                 \
      _rc = (exp);                                                       \
    } while (((_rc == -1) && ((errno == EINTR) || (errno == EAGAIN))) || \
             (_rc == -EINTR) || (_rc == -EAGAIN));                       \
    _rc;                                                                 \
  })

// std::unique_ptr doesn't let you provide a pointer to a deleter (android_logger_list_close()) if
// the type (struct logger_list) is an incomplete type, so we create ListCloser instead.
struct ListCloser {
  void operator()(struct logger_list* list) { android_logger_list_close(list); }
};

// This function is meant to be used for most log tests, it does the following:
// 1) Open the log_buffer with a blocking reader
// 2) Write the messages via write_messages
// 3) Set an alarm for 2 seconds as a timeout
// 4) Read until check_message returns true, which should be used to indicate the target message
//    is found
// 5) Open log_buffer with a non_blocking reader and dump all messages
// 6) Count the number of times check_messages returns true for these messages and assert it's
//    only 1.
template <typename FWrite, typename FCheck>
static void RunLogTests(log_id_t log_buffer, FWrite write_messages, FCheck check_message) {
  pid_t pid = getpid();

  auto logger_list = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(log_buffer, 0, 1000, pid)};
  ASSERT_TRUE(logger_list);

  write_messages();

  alarm(2);
  auto alarm_guard = android::base::make_scope_guard([] { alarm(0); });
  bool found = false;
  while (!found) {
    log_msg log_msg;
    ASSERT_GT(android_logger_list_read(logger_list.get(), &log_msg), 0);

    ASSERT_EQ(log_buffer, log_msg.id());
    ASSERT_EQ(pid, log_msg.entry.pid);

    ASSERT_NE(nullptr, log_msg.msg());

    check_message(log_msg, &found);
  }

  auto logger_list_non_block = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(log_buffer, ANDROID_LOG_NONBLOCK, 1000, pid)};
  ASSERT_TRUE(logger_list_non_block);

  size_t count = 0;
  while (true) {
    log_msg log_msg;
    auto ret = android_logger_list_read(logger_list_non_block.get(), &log_msg);
    if (ret == -EAGAIN) {
      break;
    }
    ASSERT_GT(ret, 0);

    ASSERT_EQ(log_buffer, log_msg.id());
    ASSERT_EQ(pid, log_msg.entry.pid);

    ASSERT_NE(nullptr, log_msg.msg());

    found = false;
    check_message(log_msg, &found);
    if (found) {
      ++count;
    }
  }

  EXPECT_EQ(1U, count);
}

TEST(liblog, __android_log_btwrite) {
  int intBuf = 0xDEADBEEF;
  EXPECT_LT(0,
            __android_log_btwrite(0, EVENT_TYPE_INT, &intBuf, sizeof(intBuf)));
  long long longBuf = 0xDEADBEEFA55A5AA5;
  EXPECT_LT(
      0, __android_log_btwrite(0, EVENT_TYPE_LONG, &longBuf, sizeof(longBuf)));
  char Buf[] = "\20\0\0\0DeAdBeEfA55a5aA5";
  EXPECT_LT(0,
            __android_log_btwrite(0, EVENT_TYPE_STRING, Buf, sizeof(Buf) - 1));
}

#if defined(__ANDROID__)
static std::string popenToString(const std::string& command) {
  std::string ret;

  FILE* fp = popen(command.c_str(), "re");
  if (fp) {
    if (!android::base::ReadFdToString(fileno(fp), &ret)) ret = "";
    pclose(fp);
  }
  return ret;
}

static bool isPmsgActive() {
  pid_t pid = getpid();

  std::string myPidFds =
      popenToString(android::base::StringPrintf("ls -l /proc/%d/fd", pid));
  if (myPidFds.length() == 0) return true;  // guess it is?

  return std::string::npos != myPidFds.find(" -> /dev/pmsg0");
}

static bool isLogdwActive() {
  std::string logdwSignature =
      popenToString("grep -a /dev/socket/logdw /proc/net/unix");
  size_t beginning = logdwSignature.find(' ');
  if (beginning == std::string::npos) return true;
  beginning = logdwSignature.find(' ', beginning + 1);
  if (beginning == std::string::npos) return true;
  size_t end = logdwSignature.find(' ', beginning + 1);
  if (end == std::string::npos) return true;
  end = logdwSignature.find(' ', end + 1);
  if (end == std::string::npos) return true;
  end = logdwSignature.find(' ', end + 1);
  if (end == std::string::npos) return true;
  end = logdwSignature.find(' ', end + 1);
  if (end == std::string::npos) return true;
  std::string allLogdwEndpoints = popenToString(
      "grep -a ' 00000002" + logdwSignature.substr(beginning, end - beginning) +
      " ' /proc/net/unix | " +
      "sed -n 's/.* \\([0-9][0-9]*\\)$/ -> socket:[\\1]/p'");
  if (allLogdwEndpoints.length() == 0) return true;

  // NB: allLogdwEndpoints has some false positives in it, but those
  // strangers do not overlap with the simplistic activities inside this
  // test suite.

  pid_t pid = getpid();

  std::string myPidFds =
      popenToString(android::base::StringPrintf("ls -l /proc/%d/fd", pid));
  if (myPidFds.length() == 0) return true;

  // NB: fgrep with multiple strings is broken in Android
  for (beginning = 0;
       (end = allLogdwEndpoints.find('\n', beginning)) != std::string::npos;
       beginning = end + 1) {
    if (myPidFds.find(allLogdwEndpoints.substr(beginning, end - beginning)) !=
        std::string::npos)
      return true;
  }
  return false;
}

static bool tested__android_log_close;
#endif

TEST(liblog, __android_log_btwrite__android_logger_list_read) {
#ifdef __ANDROID__
  log_time ts(CLOCK_MONOTONIC);
  log_time ts1(ts);

  bool has_pstore = access("/dev/pmsg0", W_OK) == 0;

  auto write_function = [&] {
    EXPECT_LT(0, __android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts)));
    // Check that we can close and reopen the logger
    bool logdwActiveAfter__android_log_btwrite;
    if (getuid() == AID_ROOT) {
      tested__android_log_close = true;
      if (has_pstore) {
        bool pmsgActiveAfter__android_log_btwrite = isPmsgActive();
        EXPECT_TRUE(pmsgActiveAfter__android_log_btwrite);
      }
      logdwActiveAfter__android_log_btwrite = isLogdwActive();
      EXPECT_TRUE(logdwActiveAfter__android_log_btwrite);
    } else if (!tested__android_log_close) {
      fprintf(stderr, "WARNING: can not test __android_log_close()\n");
    }
    __android_log_close();
    if (getuid() == AID_ROOT) {
      if (has_pstore) {
        bool pmsgActiveAfter__android_log_close = isPmsgActive();
        EXPECT_FALSE(pmsgActiveAfter__android_log_close);
      }
      bool logdwActiveAfter__android_log_close = isLogdwActive();
      EXPECT_FALSE(logdwActiveAfter__android_log_close);
    }

    ts1 = log_time(CLOCK_MONOTONIC);
    EXPECT_LT(0, __android_log_btwrite(0, EVENT_TYPE_LONG, &ts1, sizeof(ts1)));
    if (getuid() == AID_ROOT) {
      if (has_pstore) {
        bool pmsgActiveAfter__android_log_btwrite = isPmsgActive();
        EXPECT_TRUE(pmsgActiveAfter__android_log_btwrite);
      }
      logdwActiveAfter__android_log_btwrite = isLogdwActive();
      EXPECT_TRUE(logdwActiveAfter__android_log_btwrite);
    }
  };

  int count = 0;
  int second_count = 0;

  auto check_function = [&](log_msg log_msg, bool* found) {
    if ((log_msg.entry.len != sizeof(android_log_event_long_t)) ||
        (log_msg.id() != LOG_ID_EVENTS)) {
      return;
    }

    android_log_event_long_t* eventData;
    eventData = reinterpret_cast<android_log_event_long_t*>(log_msg.msg());

    if (!eventData || (eventData->payload.type != EVENT_TYPE_LONG)) {
      return;
    }

    log_time* tx = reinterpret_cast<log_time*>(&eventData->payload.data);
    if (ts == *tx) {
      ++count;
    } else if (ts1 == *tx) {
      ++second_count;
    }

    if (count == 1 && second_count == 1) {
      count = 0;
      second_count = 0;
      *found = true;
    }
  };

  RunLogTests(LOG_ID_EVENTS, write_function, check_function);

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __android_log_write__android_logger_list_read) {
#ifdef __ANDROID__
  pid_t pid = getpid();

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  std::string buf = android::base::StringPrintf("pid=%u ts=%ld.%09ld", pid, ts.tv_sec, ts.tv_nsec);
  static const char tag[] = "liblog.__android_log_write__android_logger_list_read";
  static const char prio = ANDROID_LOG_DEBUG;

  std::string expected_message =
      std::string(&prio, sizeof(prio)) + tag + std::string("", 1) + buf + std::string("", 1);

  auto write_function = [&] { ASSERT_LT(0, __android_log_write(prio, tag, buf.c_str())); };

  auto check_function = [&](log_msg log_msg, bool* found) {
    if (log_msg.entry.len != expected_message.length()) {
      return;
    }

    if (expected_message != std::string(log_msg.msg(), log_msg.entry.len)) {
      return;
    }

    *found = true;
  };

  RunLogTests(LOG_ID_MAIN, write_function, check_function);

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

static void bswrite_test(const char* message) {
#ifdef __ANDROID__
  pid_t pid = getpid();

  size_t num_lines = 1, size = 0, length = 0, total = 0;
  const char* cp = message;
  while (*cp) {
    if (*cp == '\n') {
      if (cp[1]) {
        ++num_lines;
      }
    } else {
      ++size;
    }
    ++cp;
    ++total;
    ++length;
    if ((LOGGER_ENTRY_MAX_PAYLOAD - 4 - 1 - 4) <= length) {
      break;
    }
  }
  while (*cp) {
    ++cp;
    ++total;
  }

  auto write_function = [&] { EXPECT_LT(0, __android_log_bswrite(0, message)); };

  auto check_function = [&](log_msg log_msg, bool* found) {
    if ((size_t)log_msg.entry.len != (sizeof(android_log_event_string_t) + length) ||
        log_msg.id() != LOG_ID_EVENTS) {
      return;
    }

    android_log_event_string_t* eventData;
    eventData = reinterpret_cast<android_log_event_string_t*>(log_msg.msg());

    if (!eventData || (eventData->type != EVENT_TYPE_STRING)) {
      return;
    }

    size_t len = eventData->length;
    if (len == total) {
      *found = true;

      AndroidLogFormat* logformat = android_log_format_new();
      EXPECT_TRUE(NULL != logformat);
      AndroidLogEntry entry;
      char msgBuf[1024];
      if (length != total) {
        fprintf(stderr, "Expect \"Binary log entry conversion failed\"\n");
      }
      int processBinaryLogBuffer = android_log_processBinaryLogBuffer(
          &log_msg.entry, &entry, nullptr, msgBuf, sizeof(msgBuf));
      EXPECT_EQ((length == total) ? 0 : -1, processBinaryLogBuffer);
      if ((processBinaryLogBuffer == 0) || entry.message) {
        size_t line_overhead = 20;
        if (pid > 99999) ++line_overhead;
        if (pid > 999999) ++line_overhead;
        fflush(stderr);
        if (processBinaryLogBuffer) {
          EXPECT_GT((int)((line_overhead * num_lines) + size),
                    android_log_printLogLine(logformat, fileno(stderr), &entry));
        } else {
          EXPECT_EQ((int)((line_overhead * num_lines) + size),
                    android_log_printLogLine(logformat, fileno(stderr), &entry));
        }
      }
      android_log_format_free(logformat);
    }
  };

  RunLogTests(LOG_ID_EVENTS, write_function, check_function);

#else
  message = NULL;
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __android_log_bswrite_and_print) {
  bswrite_test("Hello World");
}

TEST(liblog, __android_log_bswrite_and_print__empty_string) {
  bswrite_test("");
}

TEST(liblog, __android_log_bswrite_and_print__newline_prefix) {
  bswrite_test("\nHello World\n");
}

TEST(liblog, __android_log_bswrite_and_print__newline_space_prefix) {
  bswrite_test("\n Hello World \n");
}

TEST(liblog, __android_log_bswrite_and_print__multiple_newline) {
  bswrite_test("one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten");
}

static void buf_write_test(const char* message) {
#ifdef __ANDROID__
  pid_t pid = getpid();

  static const char tag[] = "TEST__android_log_buf_write";

  auto write_function = [&] {
    EXPECT_LT(0, __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_INFO, tag, message));
  };
  size_t num_lines = 1, size = 0, length = 0;
  const char* cp = message;
  while (*cp) {
    if (*cp == '\n') {
      if (cp[1]) {
        ++num_lines;
      }
    } else {
      ++size;
    }
    ++length;
    if ((LOGGER_ENTRY_MAX_PAYLOAD - 2 - sizeof(tag)) <= length) {
      break;
    }
    ++cp;
  }

  auto check_function = [&](log_msg log_msg, bool* found) {
    if ((size_t)log_msg.entry.len != (sizeof(tag) + length + 2) || log_msg.id() != LOG_ID_MAIN) {
      return;
    }

    *found = true;

    AndroidLogFormat* logformat = android_log_format_new();
    EXPECT_TRUE(NULL != logformat);
    AndroidLogEntry entry;
    int processLogBuffer = android_log_processLogBuffer(&log_msg.entry, &entry);
    EXPECT_EQ(0, processLogBuffer);
    if (processLogBuffer == 0) {
      size_t line_overhead = 11;
      if (pid > 99999) ++line_overhead;
      if (pid > 999999) ++line_overhead;
      fflush(stderr);
      EXPECT_EQ((int)(((line_overhead + sizeof(tag)) * num_lines) + size),
                android_log_printLogLine(logformat, fileno(stderr), &entry));
    }
    android_log_format_free(logformat);
  };

  RunLogTests(LOG_ID_MAIN, write_function, check_function);

#else
  message = NULL;
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __android_log_buf_write_and_print__empty) {
  buf_write_test("");
}

TEST(liblog, __android_log_buf_write_and_print__newline_prefix) {
  buf_write_test("\nHello World\n");
}

TEST(liblog, __android_log_buf_write_and_print__newline_space_prefix) {
  buf_write_test("\n Hello World \n");
}

#ifdef ENABLE_FLAKY_TESTS
#ifdef __ANDROID__
static unsigned signaled;
static log_time signal_time;

/*
 *  Strictly, we are not allowed to log messages in a signal context, but we
 * do make an effort to keep the failure surface minimized, and this in-effect
 * should catch any regressions in that effort. The odds of a logged message
 * in a signal handler causing a lockup problem should be _very_ small.
 */
static void caught_blocking_signal(int /*signum*/) {
  unsigned long long v = 0xDEADBEEFA55A0000ULL;

  v += getpid() & 0xFFFF;

  ++signaled;
  if ((signal_time.tv_sec == 0) && (signal_time.tv_nsec == 0)) {
    signal_time = log_time(CLOCK_MONOTONIC);
    signal_time.tv_sec += 2;
  }

  LOG_FAILURE_RETRY(__android_log_btwrite(0, EVENT_TYPE_LONG, &v, sizeof(v)));
}

// Fill in current process user and system time in 10ms increments
static void get_ticks(unsigned long long* uticks, unsigned long long* sticks) {
  *uticks = *sticks = 0;

  pid_t pid = getpid();

  char buffer[512];
  snprintf(buffer, sizeof(buffer), "/proc/%u/stat", pid);

  FILE* fp = fopen(buffer, "re");
  if (!fp) {
    return;
  }

  char* cp = fgets(buffer, sizeof(buffer), fp);
  fclose(fp);
  if (!cp) {
    return;
  }

  pid_t d;
  char s[sizeof(buffer)];
  char c;
  long long ll;
  unsigned long long ull;

  if (15 != sscanf(buffer,
                   "%d %s %c %lld %lld %lld %lld %lld %llu %llu %llu %llu %llu "
                   "%llu %llu ",
                   &d, s, &c, &ll, &ll, &ll, &ll, &ll, &ull, &ull, &ull, &ull,
                   &ull, uticks, sticks)) {
    *uticks = *sticks = 0;
  }
}
#endif

TEST(liblog, android_logger_list_read__cpu_signal) {
#ifdef __ANDROID__
  struct logger_list* logger_list;
  unsigned long long v = 0xDEADBEEFA55A0000ULL;

  pid_t pid = getpid();

  v += pid & 0xFFFF;

  ASSERT_TRUE(NULL != (logger_list = android_logger_list_open(LOG_ID_EVENTS, 0, 1000, pid)));

  int count = 0;

  int signals = 0;

  unsigned long long uticks_start;
  unsigned long long sticks_start;
  get_ticks(&uticks_start, &sticks_start);

  const unsigned alarm_time = 10;

  memset(&signal_time, 0, sizeof(signal_time));

  signal(SIGALRM, caught_blocking_signal);
  alarm(alarm_time);

  signaled = 0;

  do {
    log_msg log_msg;
    if (android_logger_list_read(logger_list, &log_msg) <= 0) {
      break;
    }

    alarm(alarm_time);

    ++count;

    ASSERT_EQ(log_msg.entry.pid, pid);

    if ((log_msg.entry.len != sizeof(android_log_event_long_t)) ||
        (log_msg.id() != LOG_ID_EVENTS)) {
      continue;
    }

    android_log_event_long_t* eventData;
    eventData = reinterpret_cast<android_log_event_long_t*>(log_msg.msg());

    if (!eventData || (eventData->payload.type != EVENT_TYPE_LONG)) {
      continue;
    }

    char* cp = reinterpret_cast<char*>(&eventData->payload.data);
    unsigned long long l = cp[0] & 0xFF;
    l |= (unsigned long long)(cp[1] & 0xFF) << 8;
    l |= (unsigned long long)(cp[2] & 0xFF) << 16;
    l |= (unsigned long long)(cp[3] & 0xFF) << 24;
    l |= (unsigned long long)(cp[4] & 0xFF) << 32;
    l |= (unsigned long long)(cp[5] & 0xFF) << 40;
    l |= (unsigned long long)(cp[6] & 0xFF) << 48;
    l |= (unsigned long long)(cp[7] & 0xFF) << 56;

    if (l == v) {
      ++signals;
      break;
    }
  } while (!signaled || (log_time(CLOCK_MONOTONIC) < signal_time));
  alarm(0);
  signal(SIGALRM, SIG_DFL);

  EXPECT_LE(1, count);

  EXPECT_EQ(1, signals);

  android_logger_list_close(logger_list);

  unsigned long long uticks_end;
  unsigned long long sticks_end;
  get_ticks(&uticks_end, &sticks_end);

  // Less than 1% in either user or system time, or both
  const unsigned long long one_percent_ticks = alarm_time;
  unsigned long long user_ticks = uticks_end - uticks_start;
  unsigned long long system_ticks = sticks_end - sticks_start;
  EXPECT_GT(one_percent_ticks, user_ticks);
  EXPECT_GT(one_percent_ticks, system_ticks);
  EXPECT_GT(one_percent_ticks, user_ticks + system_ticks);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

#ifdef __ANDROID__
/*
 *  Strictly, we are not allowed to log messages in a signal context, the
 * correct way to handle this is to ensure the messages are constructed in
 * a thread; the signal handler should only unblock the thread.
 */
static sem_t thread_trigger;

static void caught_blocking_thread(int /*signum*/) {
  sem_post(&thread_trigger);
}

static void* running_thread(void*) {
  unsigned long long v = 0xDEADBEAFA55A0000ULL;

  v += getpid() & 0xFFFF;

  struct timespec timeout;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += 55;
  sem_timedwait(&thread_trigger, &timeout);

  ++signaled;
  if ((signal_time.tv_sec == 0) && (signal_time.tv_nsec == 0)) {
    signal_time = log_time(CLOCK_MONOTONIC);
    signal_time.tv_sec += 2;
  }

  LOG_FAILURE_RETRY(__android_log_btwrite(0, EVENT_TYPE_LONG, &v, sizeof(v)));

  return NULL;
}

static int start_thread() {
  sem_init(&thread_trigger, 0, 0);

  pthread_attr_t attr;
  if (pthread_attr_init(&attr)) {
    return -1;
  }

  struct sched_param param;

  memset(&param, 0, sizeof(param));
  pthread_attr_setschedparam(&attr, &param);
  pthread_attr_setschedpolicy(&attr, SCHED_BATCH);

  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
    pthread_attr_destroy(&attr);
    return -1;
  }

  pthread_t thread;
  if (pthread_create(&thread, &attr, running_thread, NULL)) {
    pthread_attr_destroy(&attr);
    return -1;
  }

  pthread_attr_destroy(&attr);
  return 0;
}
#endif

TEST(liblog, android_logger_list_read__cpu_thread) {
#ifdef __ANDROID__
  struct logger_list* logger_list;
  unsigned long long v = 0xDEADBEAFA55A0000ULL;

  pid_t pid = getpid();

  v += pid & 0xFFFF;

  ASSERT_TRUE(NULL != (logger_list = android_logger_list_open(LOG_ID_EVENTS, 0, 1000, pid)));

  int count = 0;

  int signals = 0;

  unsigned long long uticks_start;
  unsigned long long sticks_start;
  get_ticks(&uticks_start, &sticks_start);

  const unsigned alarm_time = 10;

  memset(&signal_time, 0, sizeof(signal_time));

  signaled = 0;
  EXPECT_EQ(0, start_thread());

  signal(SIGALRM, caught_blocking_thread);
  alarm(alarm_time);

  do {
    log_msg log_msg;
    if (LOG_FAILURE_RETRY(android_logger_list_read(logger_list, &log_msg)) <= 0) {
      break;
    }

    alarm(alarm_time);

    ++count;

    ASSERT_EQ(log_msg.entry.pid, pid);

    if ((log_msg.entry.len != sizeof(android_log_event_long_t)) ||
        (log_msg.id() != LOG_ID_EVENTS)) {
      continue;
    }

    android_log_event_long_t* eventData;
    eventData = reinterpret_cast<android_log_event_long_t*>(log_msg.msg());

    if (!eventData || (eventData->payload.type != EVENT_TYPE_LONG)) {
      continue;
    }

    char* cp = reinterpret_cast<char*>(&eventData->payload.data);
    unsigned long long l = cp[0] & 0xFF;
    l |= (unsigned long long)(cp[1] & 0xFF) << 8;
    l |= (unsigned long long)(cp[2] & 0xFF) << 16;
    l |= (unsigned long long)(cp[3] & 0xFF) << 24;
    l |= (unsigned long long)(cp[4] & 0xFF) << 32;
    l |= (unsigned long long)(cp[5] & 0xFF) << 40;
    l |= (unsigned long long)(cp[6] & 0xFF) << 48;
    l |= (unsigned long long)(cp[7] & 0xFF) << 56;

    if (l == v) {
      ++signals;
      break;
    }
  } while (!signaled || (log_time(CLOCK_MONOTONIC) < signal_time));
  alarm(0);
  signal(SIGALRM, SIG_DFL);

  EXPECT_LE(1, count);

  EXPECT_EQ(1, signals);

  android_logger_list_close(logger_list);

  unsigned long long uticks_end;
  unsigned long long sticks_end;
  get_ticks(&uticks_end, &sticks_end);

  // Less than 1% in either user or system time, or both
  const unsigned long long one_percent_ticks = alarm_time;
  unsigned long long user_ticks = uticks_end - uticks_start;
  unsigned long long system_ticks = sticks_end - sticks_start;
  EXPECT_GT(one_percent_ticks, user_ticks);
  EXPECT_GT(one_percent_ticks, system_ticks);
  EXPECT_GT(one_percent_ticks, user_ticks + system_ticks);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
#endif  // ENABLE_FLAKY_TESTS

static const char max_payload_buf[] =
    "LEONATO\n\
I learn in this letter that Don Peter of Arragon\n\
comes this night to Messina\n\
MESSENGER\n\
He is very near by this: he was not three leagues off\n\
when I left him\n\
LEONATO\n\
How many gentlemen have you lost in this action?\n\
MESSENGER\n\
But few of any sort, and none of name\n\
LEONATO\n\
A victory is twice itself when the achiever brings\n\
home full numbers. I find here that Don Peter hath\n\
bestowed much honour on a young Florentine called Claudio\n\
MESSENGER\n\
Much deserved on his part and equally remembered by\n\
Don Pedro: he hath borne himself beyond the\n\
promise of his age, doing, in the figure of a lamb,\n\
the feats of a lion: he hath indeed better\n\
bettered expectation than you must expect of me to\n\
tell you how\n\
LEONATO\n\
He hath an uncle here in Messina will be very much\n\
glad of it.\n\
MESSENGER\n\
I have already delivered him letters, and there\n\
appears much joy in him; even so much that joy could\n\
not show itself modest enough without a badge of\n\
bitterness.\n\
LEONATO\n\
Did he break out into tears?\n\
MESSENGER\n\
In great measure.\n\
LEONATO\n\
A kind overflow of kindness: there are no faces\n\
truer than those that are so washed. How much\n\
better is it to weep at joy than to joy at weeping!\n\
BEATRICE\n\
I pray you, is Signior Mountanto returned from the\n\
wars or no?\n\
MESSENGER\n\
I know none of that name, lady: there was none such\n\
in the army of any sort.\n\
LEONATO\n\
What is he that you ask for, niece?\n\
HERO\n\
My cousin means Signior Benedick of Padua.\n\
MESSENGER\n\
O, he's returned; and as pleasant as ever he was.\n\
BEATRICE\n\
He set up his bills here in Messina and challenged\n\
Cupid at the flight; and my uncle's fool, reading\n\
the challenge, subscribed for Cupid, and challenged\n\
him at the bird-bolt. I pray you, how many hath he\n\
killed and eaten in these wars? But how many hath\n\
he killed? for indeed I promised to eat all of his killing.\n\
LEONATO\n\
Faith, niece, you tax Signior Benedick too much;\n\
but he'll be meet with you, I doubt it not.\n\
MESSENGER\n\
He hath done good service, lady, in these wars.\n\
BEATRICE\n\
You had musty victual, and he hath holp to eat it:\n\
he is a very valiant trencherman; he hath an\n\
excellent stomach.\n\
MESSENGER\n\
And a good soldier too, lady.\n\
BEATRICE\n\
And a good soldier to a lady: but what is he to a lord?\n\
MESSENGER\n\
A lord to a lord, a man to a man; stuffed with all\n\
honourable virtues.\n\
BEATRICE\n\
It is so, indeed; he is no less than a stuffed man:\n\
but for the stuffing,--well, we are all mortal.\n\
LEONATO\n\
You must not, sir, mistake my niece. There is a\n\
kind of merry war betwixt Signior Benedick and her:\n\
they never meet but there's a skirmish of wit\n\
between them.\n\
BEATRICE\n\
Alas! he gets nothing by that. In our last\n\
conflict four of his five wits went halting off, and\n\
now is the whole man governed with one: so that if\n\
he have wit enough to keep himself warm, let him\n\
bear it for a difference between himself and his\n\
horse; for it is all the wealth that he hath left,\n\
to be known a reasonable creature. Who is his\n\
companion now? He hath every month a new sworn brother.\n\
MESSENGER\n\
Is't possible?\n\
BEATRICE\n\
Very easily possible: he wears his faith but as\n\
the fashion of his hat; it ever changes with the\n\
next block.\n\
MESSENGER\n\
I see, lady, the gentleman is not in your books.\n\
BEATRICE\n\
No; an he were, I would burn my study. But, I pray\n\
you, who is his companion? Is there no young\n\
squarer now that will make a voyage with him to the devil?\n\
MESSENGER\n\
He is most in the company of the right noble Claudio.\n\
BEATRICE\n\
O Lord, he will hang upon him like a disease: he\n\
is sooner caught than the pestilence, and the taker\n\
runs presently mad. God help the noble Claudio! if\n\
he have caught the Benedick, it will cost him a\n\
thousand pound ere a' be cured.\n\
MESSENGER\n\
I will hold friends with you, lady.\n\
BEATRICE\n\
Do, good friend.\n\
LEONATO\n\
You will never run mad, niece.\n\
BEATRICE\n\
No, not till a hot January.\n\
MESSENGER\n\
Don Pedro is approached.\n\
Enter DON PEDRO, DON JOHN, CLAUDIO, BENEDICK, and BALTHASAR\n\
\n\
DON PEDRO\n\
Good Signior Leonato, you are come to meet your\n\
trouble: the fashion of the world is to avoid\n\
cost, and you encounter it\n\
LEONATO\n\
Never came trouble to my house in the likeness of your grace,\n\
for trouble being gone, comfort should remain, but\n\
when you depart from me, sorrow abides and happiness\n\
takes his leave.";

TEST(liblog, max_payload) {
#ifdef __ANDROID__
  static const char max_payload_tag[] = "TEST_max_payload_and_longish_tag_XXXX";
#define SIZEOF_MAX_PAYLOAD_BUF (LOGGER_ENTRY_MAX_PAYLOAD - sizeof(max_payload_tag) - 1)

  pid_t pid = getpid();
  char tag[sizeof(max_payload_tag)];
  memcpy(tag, max_payload_tag, sizeof(tag));
  snprintf(tag + sizeof(tag) - 5, 5, "%04X", pid & 0xFFFF);

  auto write_function = [&] {
    LOG_FAILURE_RETRY(
        __android_log_buf_write(LOG_ID_SYSTEM, ANDROID_LOG_INFO, tag, max_payload_buf));
  };

  ssize_t max_len = 0;
  auto check_function = [&](log_msg log_msg, bool* found) {
    char* data = log_msg.msg();

    if (!data || strcmp(++data, tag)) {
      return;
    }

    data += strlen(data) + 1;

    const char* left = data;
    const char* right = max_payload_buf;
    while (*left && *right && (*left == *right)) {
      ++left;
      ++right;
    }

    if (max_len <= (left - data)) {
      max_len = left - data + 1;
    }

    if (max_len > 512) {
      *found = true;
    }
  };

  RunLogTests(LOG_ID_SYSTEM, write_function, check_function);

  EXPECT_LE(SIZEOF_MAX_PAYLOAD_BUF, static_cast<size_t>(max_len));
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __android_log_buf_print__maxtag) {
#ifdef __ANDROID__
  auto write_function = [&] {
    EXPECT_LT(0, __android_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_INFO, max_payload_buf,
                                         max_payload_buf));
  };

  auto check_function = [&](log_msg log_msg, bool* found) {
    if ((size_t)log_msg.entry.len < LOGGER_ENTRY_MAX_PAYLOAD) {
      return;
    }

    *found = true;

    AndroidLogFormat* logformat = android_log_format_new();
    EXPECT_TRUE(NULL != logformat);
    AndroidLogEntry entry;
    int processLogBuffer = android_log_processLogBuffer(&log_msg.entry, &entry);
    EXPECT_EQ(0, processLogBuffer);
    if (processLogBuffer == 0) {
      fflush(stderr);
      int printLogLine =
          android_log_printLogLine(logformat, fileno(stderr), &entry);
      // Legacy tag truncation
      EXPECT_LE(128, printLogLine);
      // Measured maximum if we try to print part of the tag as message
      EXPECT_GT(LOGGER_ENTRY_MAX_PAYLOAD * 13 / 8, printLogLine);
    }
    android_log_format_free(logformat);
  };

  RunLogTests(LOG_ID_MAIN, write_function, check_function);

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

// Note: This test is tautological. android_logger_list_read() calls recv() with
// LOGGER_ENTRY_MAX_PAYLOAD as its size argument, so it's not possible for this test to read a
// payload larger than that size.
TEST(liblog, too_big_payload) {
#ifdef __ANDROID__
  pid_t pid = getpid();
  static const char big_payload_tag[] = "TEST_big_payload_XXXX";
  char tag[sizeof(big_payload_tag)];
  memcpy(tag, big_payload_tag, sizeof(tag));
  snprintf(tag + sizeof(tag) - 5, 5, "%04X", pid & 0xFFFF);

  std::string longString(3266519, 'x');
  ssize_t ret;

  auto write_function = [&] {
    ret = LOG_FAILURE_RETRY(
        __android_log_buf_write(LOG_ID_SYSTEM, ANDROID_LOG_INFO, tag, longString.c_str()));
  };

  auto check_function = [&](log_msg log_msg, bool* found) {
    char* data = log_msg.msg();

    if (!data || strcmp(++data, tag)) {
      return;
    }

    data += strlen(data) + 1;

    const char* left = data;
    const char* right = longString.c_str();
    while (*left && *right && (*left == *right)) {
      ++left;
      ++right;
    }

    ssize_t len = left - data + 1;
    // Check that we don't see any entries larger than the max payload.
    EXPECT_LE(static_cast<size_t>(len), LOGGER_ENTRY_MAX_PAYLOAD - sizeof(big_payload_tag));

    // Once we've found our expected entry, break.
    if (len == LOGGER_ENTRY_MAX_PAYLOAD - sizeof(big_payload_tag)) {
      *found = true;
    }
  };

  RunLogTests(LOG_ID_SYSTEM, write_function, check_function);

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, dual_reader) {
#ifdef __ANDROID__
  static const int expected_count1 = 25;
  static const int expected_count2 = 25;

  pid_t pid = getpid();

  auto logger_list1 = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(LOG_ID_MAIN, 0, expected_count1, pid)};
  ASSERT_TRUE(logger_list1);

  auto logger_list2 = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(LOG_ID_MAIN, 0, expected_count2, pid)};
  ASSERT_TRUE(logger_list2);

  for (int i = 25; i > 0; --i) {
    static const char fmt[] = "dual_reader %02d";
    char buffer[sizeof(fmt) + 8];
    snprintf(buffer, sizeof(buffer), fmt, i);
    LOG_FAILURE_RETRY(__android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_INFO,
                                              "liblog", buffer));
  }

  alarm(2);
  auto alarm_guard = android::base::make_scope_guard([] { alarm(0); });

  // Wait until we see all messages with the blocking reader.
  int count1 = 0;
  int count2 = 0;

  while (count1 != expected_count2 || count2 != expected_count2) {
    log_msg log_msg;
    if (count1 < expected_count1) {
      ASSERT_GT(android_logger_list_read(logger_list1.get(), &log_msg), 0);
      count1++;
    }
    if (count2 < expected_count2) {
      ASSERT_GT(android_logger_list_read(logger_list2.get(), &log_msg), 0);
      count2++;
    }
  }

  // Test again with the nonblocking reader.
  auto logger_list_non_block1 = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(LOG_ID_MAIN, ANDROID_LOG_NONBLOCK, expected_count1, pid)};
  ASSERT_TRUE(logger_list_non_block1);

  auto logger_list_non_block2 = std::unique_ptr<struct logger_list, ListCloser>{
      android_logger_list_open(LOG_ID_MAIN, ANDROID_LOG_NONBLOCK, expected_count2, pid)};
  ASSERT_TRUE(logger_list_non_block2);
  count1 = 0;
  count2 = 0;
  bool done1 = false;
  bool done2 = false;

  while (!done1 || !done2) {
    log_msg log_msg;

    if (!done1) {
      if (android_logger_list_read(logger_list_non_block1.get(), &log_msg) <= 0) {
        done1 = true;
      } else {
        ++count1;
      }
    }

    if (!done2) {
      if (android_logger_list_read(logger_list_non_block2.get(), &log_msg) <= 0) {
        done2 = true;
      } else {
        ++count2;
      }
    }
  }

  EXPECT_EQ(expected_count1, count1);
  EXPECT_EQ(expected_count2, count2);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

static bool checkPriForTag(AndroidLogFormat* p_format, const char* tag,
                           android_LogPriority pri) {
  return android_log_shouldPrintLine(p_format, tag, pri) &&
         !android_log_shouldPrintLine(p_format, tag,
                                      (android_LogPriority)(pri - 1));
}

TEST(liblog, filterRule) {
  static const char tag[] = "random";

  AndroidLogFormat* p_format = android_log_format_new();

  android_log_addFilterRule(p_format, "*:i");

  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_INFO));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) ==
              0);
  android_log_addFilterRule(p_format, "*");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_DEBUG));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) > 0);
  android_log_addFilterRule(p_format, "*:v");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_VERBOSE));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) > 0);
  android_log_addFilterRule(p_format, "*:i");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_INFO));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) ==
              0);

  android_log_addFilterRule(p_format, tag);
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_VERBOSE));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) > 0);
  android_log_addFilterRule(p_format, "random:v");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_VERBOSE));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) > 0);
  android_log_addFilterRule(p_format, "random:d");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_DEBUG));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) > 0);
  android_log_addFilterRule(p_format, "random:w");
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_WARN));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) ==
              0);

  android_log_addFilterRule(p_format, "crap:*");
  EXPECT_TRUE(checkPriForTag(p_format, "crap", ANDROID_LOG_VERBOSE));
  EXPECT_TRUE(
      android_log_shouldPrintLine(p_format, "crap", ANDROID_LOG_VERBOSE) > 0);

  // invalid expression
  EXPECT_TRUE(android_log_addFilterRule(p_format, "random:z") < 0);
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_WARN));
  EXPECT_TRUE(android_log_shouldPrintLine(p_format, tag, ANDROID_LOG_DEBUG) ==
              0);

  // Issue #550946
  EXPECT_TRUE(android_log_addFilterString(p_format, " ") == 0);
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_WARN));

  // note trailing space
  EXPECT_TRUE(android_log_addFilterString(p_format, "*:s random:d ") == 0);
  EXPECT_TRUE(checkPriForTag(p_format, tag, ANDROID_LOG_DEBUG));

  EXPECT_TRUE(android_log_addFilterString(p_format, "*:s random:z") < 0);

#if 0  // bitrot, seek update
    char defaultBuffer[512];

    android_log_formatLogLine(p_format,
        defaultBuffer, sizeof(defaultBuffer), 0, ANDROID_LOG_ERROR, 123,
        123, 123, tag, "nofile", strlen("Hello"), "Hello", NULL);

    fprintf(stderr, "%s\n", defaultBuffer);
#endif

  android_log_format_free(p_format);
}

#ifdef ENABLE_FLAKY_TESTS
TEST(liblog, is_loggable) {
#ifdef __ANDROID__
  static const char tag[] = "is_loggable";
  static const char log_namespace[] = "persist.log.tag.";
  static const size_t base_offset = 8; /* skip "persist." */
  // sizeof("string") = strlen("string") + 1
  char key[sizeof(log_namespace) + sizeof(tag) - 1];
  char hold[4][PROP_VALUE_MAX];
  static const struct {
    int level;
    char type;
  } levels[] = {
      {ANDROID_LOG_VERBOSE, 'v'}, {ANDROID_LOG_DEBUG, 'd'},
      {ANDROID_LOG_INFO, 'i'},    {ANDROID_LOG_WARN, 'w'},
      {ANDROID_LOG_ERROR, 'e'},   {ANDROID_LOG_FATAL, 'a'},
      {ANDROID_LOG_SILENT, 's'},  {-2, 'g'},  // Illegal value, resort to default
  };

  // Set up initial test condition
  memset(hold, 0, sizeof(hold));
  snprintf(key, sizeof(key), "%s%s", log_namespace, tag);
  property_get(key, hold[0], "");
  property_set(key, "");
  property_get(key + base_offset, hold[1], "");
  property_set(key + base_offset, "");
  strcpy(key, log_namespace);
  key[sizeof(log_namespace) - 2] = '\0';
  property_get(key, hold[2], "");
  property_set(key, "");
  property_get(key, hold[3], "");
  property_set(key + base_offset, "");

  // All combinations of level and defaults
  for (size_t i = 0; i < (sizeof(levels) / sizeof(levels[0])); ++i) {
    if (levels[i].level == -2) {
      continue;
    }
    for (size_t j = 0; j < (sizeof(levels) / sizeof(levels[0])); ++j) {
      if (levels[j].level == -2) {
        continue;
      }
      fprintf(stderr, "i=%zu j=%zu\r", i, j);
      bool android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), levels[j].level);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1)) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), levels[j].level));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), levels[j].level));
        }
      }
    }
  }

  // All combinations of level and tag and global properties
  for (size_t i = 0; i < (sizeof(levels) / sizeof(levels[0])); ++i) {
    if (levels[i].level == -2) {
      continue;
    }
    for (size_t j = 0; j < (sizeof(levels) / sizeof(levels[0])); ++j) {
      char buf[2];
      buf[0] = levels[j].type;
      buf[1] = '\0';

      snprintf(key, sizeof(key), "%s%s", log_namespace, tag);
      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j, key,
              buf);
      usleep(20000);
      property_set(key, buf);
      bool android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_DEBUG) && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key, "");

      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j,
              key + base_offset, buf);
      property_set(key + base_offset, buf);
      android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_DEBUG) && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key + base_offset, "");

      strcpy(key, log_namespace);
      key[sizeof(log_namespace) - 2] = '\0';
      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j, key,
              buf);
      property_set(key, buf);
      android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_DEBUG) && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key, "");

      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j,
              key + base_offset, buf);
      property_set(key + base_offset, buf);
      android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_DEBUG) && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key + base_offset, "");
    }
  }

  // All combinations of level and tag properties, but with global set to INFO
  strcpy(key, log_namespace);
  key[sizeof(log_namespace) - 2] = '\0';
  usleep(20000);
  property_set(key, "I");
  snprintf(key, sizeof(key), "%s%s", log_namespace, tag);
  for (size_t i = 0; i < (sizeof(levels) / sizeof(levels[0])); ++i) {
    if (levels[i].level == -2) {
      continue;
    }
    for (size_t j = 0; j < (sizeof(levels) / sizeof(levels[0])); ++j) {
      char buf[2];
      buf[0] = levels[j].type;
      buf[1] = '\0';

      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j, key,
              buf);
      usleep(20000);
      property_set(key, buf);
      bool android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_INFO)  // Yes INFO
           && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key, "");

      fprintf(stderr, "i=%zu j=%zu property_set(\"%s\",\"%s\")\r", i, j,
              key + base_offset, buf);
      property_set(key + base_offset, buf);
      android_log_is_loggable = __android_log_is_loggable_len(
          levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG);
      if ((levels[i].level < levels[j].level) || (levels[j].level == -1) ||
          ((levels[i].level < ANDROID_LOG_INFO)  // Yes INFO
           && (levels[j].level == -2))) {
        if (android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_FALSE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_FALSE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      } else {
        if (!android_log_is_loggable) {
          fprintf(stderr, "\n");
        }
        EXPECT_TRUE(android_log_is_loggable);
        for (size_t k = 10; k; --k) {
          EXPECT_TRUE(__android_log_is_loggable_len(
              levels[i].level, tag, strlen(tag), ANDROID_LOG_DEBUG));
        }
      }
      usleep(20000);
      property_set(key + base_offset, "");
    }
  }

  // reset parms
  snprintf(key, sizeof(key), "%s%s", log_namespace, tag);
  usleep(20000);
  property_set(key, hold[0]);
  property_set(key + base_offset, hold[1]);
  strcpy(key, log_namespace);
  key[sizeof(log_namespace) - 2] = '\0';
  property_set(key, hold[2]);
  property_set(key + base_offset, hold[3]);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
#endif  // ENABLE_FLAKY_TESTS

#ifdef ENABLE_FLAKY_TESTS
// Following tests the specific issues surrounding error handling wrt logd.
// Kills logd and toss all collected data, equivalent to logcat -b all -c,
// except we also return errors to the logging callers.
#ifdef __ANDROID__
// helper to liblog.enoent to count end-to-end matching logging messages.
static int count_matching_ts(log_time ts) {
  usleep(1000000);

  pid_t pid = getpid();

  struct logger_list* logger_list =
      android_logger_list_open(LOG_ID_EVENTS, ANDROID_LOG_NONBLOCK, 1000, pid);

  int count = 0;
  if (logger_list == NULL) return count;

  for (;;) {
    log_msg log_msg;
    if (android_logger_list_read(logger_list, &log_msg) <= 0) break;

    if (log_msg.entry.len != sizeof(android_log_event_long_t)) continue;
    if (log_msg.id() != LOG_ID_EVENTS) continue;

    android_log_event_long_t* eventData;
    eventData = reinterpret_cast<android_log_event_long_t*>(log_msg.msg());
    if (!eventData) continue;
    if (eventData->payload.type != EVENT_TYPE_LONG) continue;

    log_time tx(reinterpret_cast<char*>(&eventData->payload.data));
    if (ts != tx) continue;

    // found event message with matching timestamp signature in payload
    ++count;
  }
  android_logger_list_close(logger_list);

  return count;
}

TEST(liblog, enoent) {
#ifdef __ANDROID__
  if (getuid() != 0) {
    GTEST_SKIP() << "Skipping test, must be run as root.";
    return;
  }

  log_time ts(CLOCK_MONOTONIC);
  EXPECT_LT(0, __android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts)));
  EXPECT_EQ(1, count_matching_ts(ts));

  // This call will fail unless we are root, beware of any
  // test prior to this one playing with setuid and causing interference.
  // We need to run before these tests so that they do not interfere with
  // this test.
  //
  // Stopping the logger can affect some other test's expectations as they
  // count on the log buffers filled with existing content, and this
  // effectively does a logcat -c emptying it.  So we want this test to be
  // as near as possible to the bottom of the file.  For example
  // liblog.android_logger_get_ is one of those tests that has no recourse
  // and that would be adversely affected by emptying the log if it was run
  // right after this test.
  system("stop logd");
  usleep(1000000);

  // A clean stop like we are testing returns -ENOENT, but in the _real_
  // world we could get -ENOTCONN or -ECONNREFUSED depending on timing.
  // Alas we can not test these other return values; accept that they
  // are treated equally within the open-retry logic in liblog.
  ts = log_time(CLOCK_MONOTONIC);
  int ret = __android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts));
  std::string content = android::base::StringPrintf(
      "__android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts)) = %d %s\n",
      ret, (ret <= 0) ? strerror(-ret) : "(content sent)");
  EXPECT_TRUE(ret == -ENOENT || ret == -ENOTCONN || ret == -ECONNREFUSED) << content;
  ret = __android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts));
  content = android::base::StringPrintf(
      "__android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts)) = %d %s\n",
      ret, (ret <= 0) ? strerror(-ret) : "(content sent)");
  EXPECT_TRUE(ret == -ENOENT || ret == -ENOTCONN || ret == -ECONNREFUSED) << content;
  EXPECT_EQ(0, count_matching_ts(ts));

  system("start logd");
  usleep(1000000);

  EXPECT_EQ(0, count_matching_ts(ts));

  ts = log_time(CLOCK_MONOTONIC);
  EXPECT_LT(0, __android_log_btwrite(0, EVENT_TYPE_LONG, &ts, sizeof(ts)));
  EXPECT_EQ(1, count_matching_ts(ts));

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
#endif  // __ANDROID__
#endif  // ENABLE_FLAKY_TESTS

// Below this point we run risks of setuid(AID_SYSTEM) which may affect others.

#ifdef ENABLE_FLAKY_TESTS
// Do not retest properties, and cannot log into LOG_ID_SECURITY
TEST(liblog, __security) {
#ifdef __ANDROID__
  static const char persist_key[] = "persist.logd.security";
  static const char readonly_key[] = "ro.organization_owned";
  // A silly default value that can never be in readonly_key so
  // that it can be determined the property is not set.
  static const char nothing_val[] = "_NOTHING_TO_SEE_HERE_";
  char persist[PROP_VALUE_MAX];
  char persist_hold[PROP_VALUE_MAX];
  char readonly[PROP_VALUE_MAX];

  // First part of this test requires the test itself to have the appropriate
  // permissions. If we do not have them, we can not override them, so we
  // bail rather than give a failing grade.
  property_get(persist_key, persist, "");
  fprintf(stderr, "INFO: getprop %s -> %s\n", persist_key, persist);
  strncpy(persist_hold, persist, PROP_VALUE_MAX);
  property_get(readonly_key, readonly, nothing_val);
  fprintf(stderr, "INFO: getprop %s -> %s\n", readonly_key, readonly);

  if (!strcmp(readonly, nothing_val)) {
    // Lets check if we can set the value (we should not be allowed to do so)
    EXPECT_FALSE(__android_log_security());
    fprintf(stderr, "WARNING: setting ro.organization_owned to a domain\n");
    static const char domain[] = "com.google.android.SecOps.DeviceOwner";
    EXPECT_NE(0, property_set(readonly_key, domain));
    useconds_t total_time = 0;
    static const useconds_t seconds = 1000000;
    static const useconds_t max_time = 5 * seconds;  // not going to happen
    static const useconds_t rest = 20 * 1000;
    for (; total_time < max_time; total_time += rest) {
      usleep(rest);  // property system does not guarantee performance.
      property_get(readonly_key, readonly, nothing_val);
      if (!strcmp(readonly, domain)) {
        if (total_time > rest) {
          fprintf(stderr, "INFO: took %u.%06u seconds to set property\n",
                  (unsigned)(total_time / seconds),
                  (unsigned)(total_time % seconds));
        }
        break;
      }
    }
    EXPECT_STRNE(domain, readonly);
  }

  if (!strcasecmp(readonly, "false") || !readonly[0] ||
      !strcmp(readonly, nothing_val)) {
    // not enough permissions to run tests surrounding persist.logd.security
    EXPECT_FALSE(__android_log_security());
    return;
  }

  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, "TRUE");
  property_get(persist_key, persist, "");
  uid_t uid = getuid();
  gid_t gid = getgid();
  bool perm = (gid == AID_ROOT) || (uid == AID_ROOT);
  EXPECT_STREQ(perm ? "TRUE" : persist_hold, persist);
  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, "FALSE");
  property_get(persist_key, persist, "");
  EXPECT_STREQ(perm ? "FALSE" : persist_hold, persist);
  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, "true");
  property_get(persist_key, persist, "");
  EXPECT_STREQ(perm ? "true" : persist_hold, persist);
  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, "false");
  property_get(persist_key, persist, "");
  EXPECT_STREQ(perm ? "false" : persist_hold, persist);
  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, "");
  property_get(persist_key, persist, "");
  EXPECT_STREQ(perm ? "" : persist_hold, persist);
  if (!strcasecmp(persist, "true")) {
    EXPECT_TRUE(__android_log_security());
  } else {
    EXPECT_FALSE(__android_log_security());
  }
  property_set(persist_key, persist_hold);
  property_get(persist_key, persist, "");
  EXPECT_STREQ(persist_hold, persist);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __security_buffer) {
#ifdef __ANDROID__
  struct logger_list* logger_list;
  android_event_long_t buffer;

  static const char persist_key[] = "persist.logd.security";
  char persist[PROP_VALUE_MAX];
  bool set_persist = false;
  bool allow_security = false;

  if (__android_log_security()) {
    allow_security = true;
  } else {
    property_get(persist_key, persist, "");
    if (strcasecmp(persist, "true")) {
      property_set(persist_key, "TRUE");
      if (__android_log_security()) {
        allow_security = true;
        set_persist = true;
      } else {
        property_set(persist_key, persist);
      }
    }
  }

  if (!allow_security) {
    fprintf(stderr,
            "WARNING: "
            "security buffer disabled, bypassing end-to-end test\n");

    log_time ts(CLOCK_MONOTONIC);

    buffer.type = EVENT_TYPE_LONG;
    buffer.data = *(static_cast<uint64_t*>((void*)&ts));

    // expect failure!
    ASSERT_GE(0, __android_log_security_bwrite(0, &buffer, sizeof(buffer)));

    return;
  }

  /* Matches clientHasLogCredentials() in logd */
  uid_t uid = getuid();
  gid_t gid = getgid();
  bool clientHasLogCredentials = true;
  if ((uid != AID_SYSTEM) && (uid != AID_ROOT) && (uid != AID_LOG) &&
      (gid != AID_SYSTEM) && (gid != AID_ROOT) && (gid != AID_LOG)) {
    uid_t euid = geteuid();
    if ((euid != AID_SYSTEM) && (euid != AID_ROOT) && (euid != AID_LOG)) {
      gid_t egid = getegid();
      if ((egid != AID_SYSTEM) && (egid != AID_ROOT) && (egid != AID_LOG)) {
        int num_groups = getgroups(0, NULL);
        if (num_groups > 0) {
          gid_t groups[num_groups];
          num_groups = getgroups(num_groups, groups);
          while (num_groups > 0) {
            if (groups[num_groups - 1] == AID_LOG) {
              break;
            }
            --num_groups;
          }
        }
        if (num_groups <= 0) {
          clientHasLogCredentials = false;
        }
      }
    }
  }
  if (!clientHasLogCredentials) {
    fprintf(stderr,
            "WARNING: "
            "not in system context, bypassing end-to-end test\n");

    log_time ts(CLOCK_MONOTONIC);

    buffer.type = EVENT_TYPE_LONG;
    buffer.data = *(static_cast<uint64_t*>((void*)&ts));

    // expect failure!
    ASSERT_GE(0, __android_log_security_bwrite(0, &buffer, sizeof(buffer)));

    return;
  }

  EXPECT_EQ(0, setuid(AID_SYSTEM));  // only one that can read security buffer

  uid = getuid();
  gid = getgid();
  pid_t pid = getpid();

  ASSERT_TRUE(NULL != (logger_list = android_logger_list_open(LOG_ID_SECURITY, ANDROID_LOG_NONBLOCK,
                                                              1000, pid)));

  log_time ts(CLOCK_MONOTONIC);

  buffer.type = EVENT_TYPE_LONG;
  buffer.data = *(static_cast<uint64_t*>((void*)&ts));

  ASSERT_LT(0, __android_log_security_bwrite(0, &buffer, sizeof(buffer)));
  usleep(1000000);

  int count = 0;

  for (;;) {
    log_msg log_msg;
    if (android_logger_list_read(logger_list, &log_msg) <= 0) {
      break;
    }

    ASSERT_EQ(log_msg.entry.pid, pid);

    if ((log_msg.entry.len != sizeof(android_log_event_long_t)) ||
        (log_msg.id() != LOG_ID_SECURITY)) {
      continue;
    }

    android_log_event_long_t* eventData;
    eventData = reinterpret_cast<android_log_event_long_t*>(log_msg.msg());

    if (!eventData || (eventData->payload.type != EVENT_TYPE_LONG)) {
      continue;
    }

    log_time tx(reinterpret_cast<char*>(&eventData->payload.data));
    if (ts == tx) {
      ++count;
    }
  }

  if (set_persist) {
    property_set(persist_key, persist);
  }

  android_logger_list_close(logger_list);

  bool clientHasSecurityCredentials = (uid == AID_SYSTEM) || (gid == AID_SYSTEM);
  if (!clientHasSecurityCredentials) {
    fprintf(stderr,
            "WARNING: "
            "not system, content submitted but can not check end-to-end\n");
  }
  EXPECT_EQ(clientHasSecurityCredentials ? 1 : 0, count);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
#endif  // ENABLE_FLAKY_TESTS

#ifdef __ANDROID__
static void android_errorWriteWithInfoLog_helper(int tag, const char* subtag, int uid,
                                                 const char* payload, int data_len) {
  auto write_function = [&] {
    int ret = android_errorWriteWithInfoLog(tag, subtag, uid, payload, data_len);
    ASSERT_LT(0, ret);
  };

  auto check_function = [&](log_msg log_msg, bool* found) {
    char* event_data = log_msg.msg();
    char* original = event_data;

    // Tag
    auto* event_header = reinterpret_cast<android_event_header_t*>(event_data);
    event_data += sizeof(android_event_header_t);
    if (event_header->tag != tag) {
      return;
    }

    // List type
    auto* event_list = reinterpret_cast<android_event_list_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_LIST, event_list->type);
    ASSERT_EQ(3, event_list->element_count);
    event_data += sizeof(android_event_list_t);

    // Element #1: string type for subtag
    auto* event_string_subtag = reinterpret_cast<android_event_string_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_STRING, event_string_subtag->type);
    int32_t subtag_len = strlen(subtag);
    if (subtag_len > 32) {
      subtag_len = 32;
    }
    ASSERT_EQ(subtag_len, event_string_subtag->length);
    if (memcmp(subtag, &event_string_subtag->data, subtag_len)) {
      return;
    }
    event_data += sizeof(android_event_string_t) + subtag_len;

    // Element #2: int type for uid
    auto* event_int_uid = reinterpret_cast<android_event_int_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_INT, event_int_uid->type);
    ASSERT_EQ(uid, event_int_uid->data);
    event_data += sizeof(android_event_int_t);

    // Element #3: string type for data
    auto* event_string_data = reinterpret_cast<android_event_string_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_STRING, event_string_data->type);
    int32_t message_data_len = event_string_data->length;
    if (data_len < 512) {
      ASSERT_EQ(data_len, message_data_len);
    }
    if (memcmp(payload, &event_string_data->data, message_data_len) != 0) {
      return;
    }
    event_data += sizeof(android_event_string_t);

    if (data_len >= 512) {
      event_data += message_data_len;
      // 4 bytes for the tag, and max_payload_buf should be truncated.
      ASSERT_LE(4 + 512, event_data - original);       // worst expectations
      ASSERT_GT(4 + data_len, event_data - original);  // must be truncated
    }
    *found = true;
  };

  RunLogTests(LOG_ID_EVENTS, write_function, check_function);
}
#endif

// Make multiple tests and re-tests orthogonal to prevent falsing.
#ifdef TEST_LOGGER
#define UNIQUE_TAG(X) \
  (0x12340000 + (((X) + sizeof(int) + sizeof(void*)) << 8) + TEST_LOGGER)
#else
#define UNIQUE_TAG(X) \
  (0x12340000 + (((X) + sizeof(int) + sizeof(void*)) << 8) + 0xBA)
#endif

TEST(liblog, android_errorWriteWithInfoLog__android_logger_list_read__typical) {
#ifdef __ANDROID__
  android_errorWriteWithInfoLog_helper(UNIQUE_TAG(1), "test-subtag", -1, max_payload_buf, 200);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog,
     android_errorWriteWithInfoLog__android_logger_list_read__data_too_large) {
#ifdef __ANDROID__
  android_errorWriteWithInfoLog_helper(UNIQUE_TAG(2), "test-subtag", -1, max_payload_buf,
                                       sizeof(max_payload_buf));
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog,
     android_errorWriteWithInfoLog__android_logger_list_read__null_data) {
#ifdef __ANDROID__
  int retval_android_errorWriteWithinInfoLog =
      android_errorWriteWithInfoLog(UNIQUE_TAG(3), "test-subtag", -1, nullptr, 200);
  ASSERT_GT(0, retval_android_errorWriteWithinInfoLog);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog,
     android_errorWriteWithInfoLog__android_logger_list_read__subtag_too_long) {
#ifdef __ANDROID__
  android_errorWriteWithInfoLog_helper(
      UNIQUE_TAG(4), "abcdefghijklmnopqrstuvwxyz now i know my abc", -1, max_payload_buf, 200);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, __android_log_bswrite_and_print___max) {
  bswrite_test(max_payload_buf);
}

TEST(liblog, __android_log_buf_write_and_print__max) {
  buf_write_test(max_payload_buf);
}

TEST(liblog, android_errorWriteLog__android_logger_list_read__success) {
#ifdef __ANDROID__
  int kTag = UNIQUE_TAG(5);
  const char* kSubTag = "test-subtag";

  auto write_function = [&] {
    int retval_android_errorWriteLog = android_errorWriteLog(kTag, kSubTag);
    ASSERT_LT(0, retval_android_errorWriteLog);
  };

  auto check_function = [&](log_msg log_msg, bool* found) {
    char* event_data = log_msg.msg();

    // Tag
    auto* event_header = reinterpret_cast<android_event_header_t*>(event_data);
    event_data += sizeof(android_event_header_t);
    if (event_header->tag != kTag) {
      return;
    }

    // List type
    auto* event_list = reinterpret_cast<android_event_list_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_LIST, event_list->type);
    ASSERT_EQ(3, event_list->element_count);
    event_data += sizeof(android_event_list_t);

    // Element #1: string type for subtag
    auto* event_string_subtag = reinterpret_cast<android_event_string_t*>(event_data);
    ASSERT_EQ(EVENT_TYPE_STRING, event_string_subtag->type);
    int32_t subtag_len = strlen(kSubTag);
    ASSERT_EQ(subtag_len, event_string_subtag->length);
    if (memcmp(kSubTag, &event_string_subtag->data, subtag_len) == 0) {
      *found = true;
    }
  };

  RunLogTests(LOG_ID_EVENTS, write_function, check_function);

#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, android_errorWriteLog__android_logger_list_read__null_subtag) {
#ifdef __ANDROID__
  EXPECT_LT(android_errorWriteLog(UNIQUE_TAG(6), nullptr), 0);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

// Do not retest logger list handling
#ifdef __ANDROID__
static int is_real_element(int type) {
  return ((type == EVENT_TYPE_INT) || (type == EVENT_TYPE_LONG) ||
          (type == EVENT_TYPE_STRING) || (type == EVENT_TYPE_FLOAT));
}

static int android_log_buffer_to_string(const char* msg, size_t len,
                                        char* strOut, size_t strOutLen) {
  android_log_context context = create_android_log_parser(msg, len);
  android_log_list_element elem;
  bool overflow = false;
  /* Reserve 1 byte for null terminator. */
  size_t origStrOutLen = strOutLen--;

  if (!context) {
    return -EBADF;
  }

  memset(&elem, 0, sizeof(elem));

  size_t outCount;

  do {
    elem = android_log_read_next(context);
    switch ((int)elem.type) {
      case EVENT_TYPE_LIST:
        if (strOutLen == 0) {
          overflow = true;
        } else {
          *strOut++ = '[';
          strOutLen--;
        }
        break;

      case EVENT_TYPE_LIST_STOP:
        if (strOutLen == 0) {
          overflow = true;
        } else {
          *strOut++ = ']';
          strOutLen--;
        }
        break;

      case EVENT_TYPE_INT:
        /*
         * snprintf also requires room for the null terminator, which
         * we don't care about  but we have allocated enough room for
         * that
         */
        outCount = snprintf(strOut, strOutLen + 1, "%" PRId32, elem.data.int32);
        if (outCount <= strOutLen) {
          strOut += outCount;
          strOutLen -= outCount;
        } else {
          overflow = true;
        }
        break;

      case EVENT_TYPE_LONG:
        /*
         * snprintf also requires room for the null terminator, which
         * we don't care about but we have allocated enough room for
         * that
         */
        outCount = snprintf(strOut, strOutLen + 1, "%" PRId64, elem.data.int64);
        if (outCount <= strOutLen) {
          strOut += outCount;
          strOutLen -= outCount;
        } else {
          overflow = true;
        }
        break;

      case EVENT_TYPE_FLOAT:
        /*
         * snprintf also requires room for the null terminator, which
         * we don't care about but we have allocated enough room for
         * that
         */
        outCount = snprintf(strOut, strOutLen + 1, "%f", elem.data.float32);
        if (outCount <= strOutLen) {
          strOut += outCount;
          strOutLen -= outCount;
        } else {
          overflow = true;
        }
        break;

      default:
        elem.complete = true;
        break;

      case EVENT_TYPE_UNKNOWN:
#if 0  // Ideal purity in the test, we want to complain about UNKNOWN showing up
            if (elem.complete) {
                break;
            }
#endif
        elem.data.string = const_cast<char*>("<unknown>");
        elem.len = strlen(elem.data.string);
        FALLTHROUGH_INTENDED;
      case EVENT_TYPE_STRING:
        if (elem.len <= strOutLen) {
          memcpy(strOut, elem.data.string, elem.len);
          strOut += elem.len;
          strOutLen -= elem.len;
        } else if (strOutLen > 0) {
          /* copy what we can */
          memcpy(strOut, elem.data.string, strOutLen);
          strOut += strOutLen;
          strOutLen = 0;
          overflow = true;
        }
        break;
    }

    if (elem.complete) {
      break;
    }
    /* Determine whether to put a comma or not. */
    if (!overflow &&
        (is_real_element(elem.type) || (elem.type == EVENT_TYPE_LIST_STOP))) {
      android_log_list_element next = android_log_peek_next(context);
      if (!next.complete &&
          (is_real_element(next.type) || (next.type == EVENT_TYPE_LIST))) {
        if (strOutLen == 0) {
          overflow = true;
        } else {
          *strOut++ = ',';
          strOutLen--;
        }
      }
    }
  } while ((elem.type != EVENT_TYPE_UNKNOWN) && !overflow && !elem.complete);

  android_log_destroy(&context);

  if (overflow) {
    if (strOutLen < origStrOutLen) {
      /* leave an indicator */
      *(strOut - 1) = '!';
    } else {
      /* nothing was written at all */
      *strOut++ = '!';
    }
  }
  *strOut++ = '\0';

  if ((elem.type == EVENT_TYPE_UNKNOWN) && !elem.complete) {
    fprintf(stderr, "Binary log entry conversion failed\n");
    return -EINVAL;
  }

  return 0;
}
#endif  // __ANDROID__

#ifdef __ANDROID__
static const char* event_test_int32(uint32_t tag, size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_int32(ctx, 0x40302010));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t);

  return "1076895760";
}

static const char* event_test_int64(uint32_t tag, size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_int64(ctx, 0x8070605040302010));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint64_t);

  return "-9191740941672636400";
}

static const char* event_test_list_int64(uint32_t tag, size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int64(ctx, 0x8070605040302010));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint8_t) + sizeof(uint64_t);

  return "[-9191740941672636400]";
}

static const char* event_test_simple_automagic_list(uint32_t tag,
                                                    size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  // The convenience API where we allow a simple list to be
  // created without explicit begin or end calls.
  EXPECT_LE(0, android_log_write_int32(ctx, 0x40302010));
  EXPECT_LE(0, android_log_write_int64(ctx, 0x8070605040302010));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) +
                 sizeof(uint64_t);

  return "[1076895760,-9191740941672636400]";
}

static const char* event_test_list_empty(uint32_t tag, size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t);

  return "[]";
}

static const char* event_test_complex_nested_list(uint32_t tag,
                                                  size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }

  EXPECT_LE(0, android_log_write_list_begin(ctx));  // [
  EXPECT_LE(0, android_log_write_int32(ctx, 0x01020304));
  EXPECT_LE(0, android_log_write_int64(ctx, 0x0102030405060708));
  EXPECT_LE(0, android_log_write_string8(ctx, "Hello World"));
  EXPECT_LE(0, android_log_write_list_begin(ctx));  // [
  EXPECT_LE(0, android_log_write_int32(ctx, 1));
  EXPECT_LE(0, android_log_write_int32(ctx, 2));
  EXPECT_LE(0, android_log_write_int32(ctx, 3));
  EXPECT_LE(0, android_log_write_int32(ctx, 4));
  EXPECT_LE(0, android_log_write_list_end(ctx));  // ]
  EXPECT_LE(0, android_log_write_float32(ctx, 1.0102030405060708));
  EXPECT_LE(0, android_log_write_list_end(ctx));  // ]

  //
  // This one checks for the automagic list creation because a list
  // begin and end was missing for it! This is actually an <oops> corner
  // case, and not the behavior we morally support. The automagic API is to
  // allow for a simple case of a series of objects in a single list. e.g.
  //   int32,int32,int32,string -> [int32,int32,int32,string]
  //
  EXPECT_LE(0, android_log_write_string8(ctx, "dlroW olleH"));

  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint64_t) +
                 sizeof(uint8_t) + sizeof(uint32_t) + sizeof("Hello World") -
                 1 + sizeof(uint8_t) + sizeof(uint8_t) +
                 4 * (sizeof(uint8_t) + sizeof(uint32_t)) + sizeof(uint8_t) +
                 sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t) +
                 sizeof("dlroW olleH") - 1;

  return "[[16909060,72623859790382856,Hello World,[1,2,3,4],1.010203],dlroW "
         "olleH]";
}

static const char* event_test_7_level_prefix(uint32_t tag,
                                             size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 1));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 2));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 3));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 4));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 5));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 6));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 7));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + 7 * (sizeof(uint8_t) + sizeof(uint8_t) +
                                         sizeof(uint8_t) + sizeof(uint32_t));

  return "[[[[[[[1],2],3],4],5],6],7]";
}

static const char* event_test_7_level_suffix(uint32_t tag,
                                             size_t& expected_len) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(tag)));
  if (!ctx) {
    return NULL;
  }
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 1));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 2));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 3));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 4));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 5));
  EXPECT_LE(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_write_int32(ctx, 6));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list_end(ctx));
  EXPECT_LE(0, android_log_write_list(ctx, LOG_ID_EVENTS));
  EXPECT_LE(0, android_log_destroy(&ctx));
  EXPECT_TRUE(NULL == ctx);

  expected_len = sizeof(uint32_t) + 6 * (sizeof(uint8_t) + sizeof(uint8_t) +
                                         sizeof(uint8_t) + sizeof(uint32_t));

  return "[1,[2,[3,[4,[5,[6]]]]]]";
}

static const char* event_test_android_log_error_write(uint32_t tag,
                                                      size_t& expected_len) {
  EXPECT_LE(
      0, __android_log_error_write(tag, "Hello World", 42, "dlroW olleH", 11));

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint8_t) + sizeof(uint32_t) + sizeof("Hello World") -
                 1 + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) +
                 sizeof(uint32_t) + sizeof("dlroW olleH") - 1;

  return "[Hello World,42,dlroW olleH]";
}

static const char* event_test_android_log_error_write_null(uint32_t tag,
                                                           size_t& expected_len) {
  EXPECT_LE(0, __android_log_error_write(tag, "Hello World", 42, NULL, 0));

  expected_len = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) +
                 sizeof(uint8_t) + sizeof(uint32_t) + sizeof("Hello World") -
                 1 + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) +
                 sizeof(uint32_t) + sizeof("") - 1;

  return "[Hello World,42,]";
}

// make sure all user buffers are flushed
static void print_barrier() {
  std::cout.flush();
  fflush(stdout);
  std::cerr.flush();
  fflush(stderr);  // everything else is paranoia ...
}

static void create_android_logger(const char* (*fn)(uint32_t tag,
                                                    size_t& expected_len)) {
  size_t expected_len;
  const char* expected_string;
  auto write_function = [&] {
    expected_string = (*fn)(1005, expected_len);
    ASSERT_NE(nullptr, expected_string);
  };

  pid_t pid = getpid();
  auto check_function = [&](log_msg log_msg, bool* found) {
    if (static_cast<size_t>(log_msg.entry.len) != expected_len) {
      return;
    }

    char* eventData = log_msg.msg();

    AndroidLogFormat* logformat = android_log_format_new();
    EXPECT_TRUE(NULL != logformat);
    AndroidLogEntry entry;
    char msgBuf[1024];
    int processBinaryLogBuffer =
        android_log_processBinaryLogBuffer(&log_msg.entry, &entry, nullptr, msgBuf, sizeof(msgBuf));
    EXPECT_EQ(0, processBinaryLogBuffer);
    if (processBinaryLogBuffer == 0) {
      int line_overhead = 20;
      if (pid > 99999) ++line_overhead;
      if (pid > 999999) ++line_overhead;
      print_barrier();
      int printLogLine =
          android_log_printLogLine(logformat, fileno(stderr), &entry);
      print_barrier();
      EXPECT_EQ(line_overhead + (int)strlen(expected_string), printLogLine);
    }
    android_log_format_free(logformat);

    // test buffer reading API
    int buffer_to_string = -1;
    if (eventData) {
      auto* event_header = reinterpret_cast<android_event_header_t*>(eventData);
      eventData += sizeof(android_event_header_t);
      snprintf(msgBuf, sizeof(msgBuf), "I/[%" PRId32 "]", event_header->tag);
      print_barrier();
      fprintf(stderr, "%-10s(%5u): ", msgBuf, pid);
      memset(msgBuf, 0, sizeof(msgBuf));
      buffer_to_string =
          android_log_buffer_to_string(eventData, log_msg.entry.len, msgBuf, sizeof(msgBuf));
      fprintf(stderr, "%s\n", msgBuf);
      print_barrier();
    }
    EXPECT_EQ(0, buffer_to_string);
    EXPECT_STREQ(expected_string, msgBuf);
    *found = true;
  };

  RunLogTests(LOG_ID_EVENTS, write_function, check_function);
}
#endif

TEST(liblog, create_android_logger_int32) {
#ifdef __ANDROID__
  create_android_logger(event_test_int32);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_int64) {
#ifdef __ANDROID__
  create_android_logger(event_test_int64);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_list_int64) {
#ifdef __ANDROID__
  create_android_logger(event_test_list_int64);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_simple_automagic_list) {
#ifdef __ANDROID__
  create_android_logger(event_test_simple_automagic_list);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_list_empty) {
#ifdef __ANDROID__
  create_android_logger(event_test_list_empty);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_complex_nested_list) {
#ifdef __ANDROID__
  create_android_logger(event_test_complex_nested_list);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_7_level_prefix) {
#ifdef __ANDROID__
  create_android_logger(event_test_7_level_prefix);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_7_level_suffix) {
#ifdef __ANDROID__
  create_android_logger(event_test_7_level_suffix);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_android_log_error_write) {
#ifdef __ANDROID__
  create_android_logger(event_test_android_log_error_write);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_android_log_error_write_null) {
#ifdef __ANDROID__
  create_android_logger(event_test_android_log_error_write_null);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

TEST(liblog, create_android_logger_overflow) {
  android_log_context ctx;

  EXPECT_TRUE(NULL != (ctx = create_android_logger(1005)));
  if (ctx) {
    for (size_t i = 0; i < ANDROID_MAX_LIST_NEST_DEPTH; ++i) {
      EXPECT_LE(0, android_log_write_list_begin(ctx));
    }
    EXPECT_GT(0, android_log_write_list_begin(ctx));
    /* One more for good measure, must be permanently unhappy */
    EXPECT_GT(0, android_log_write_list_begin(ctx));
    EXPECT_LE(0, android_log_destroy(&ctx));
    EXPECT_TRUE(NULL == ctx);
  }

  ASSERT_TRUE(NULL != (ctx = create_android_logger(1005)));
  for (size_t i = 0; i < ANDROID_MAX_LIST_NEST_DEPTH; ++i) {
    EXPECT_LE(0, android_log_write_list_begin(ctx));
    EXPECT_LE(0, android_log_write_int32(ctx, i));
  }
  EXPECT_GT(0, android_log_write_list_begin(ctx));
  /* One more for good measure, must be permanently unhappy */
  EXPECT_GT(0, android_log_write_list_begin(ctx));
  EXPECT_LE(0, android_log_destroy(&ctx));
  ASSERT_TRUE(NULL == ctx);
}

#ifdef ENABLE_FLAKY_TESTS
#ifdef __ANDROID__
#ifndef NO_PSTORE
static const char __pmsg_file[] =
    "/data/william-shakespeare/MuchAdoAboutNothing.txt";
#endif /* NO_PSTORE */
#endif

TEST(liblog, __android_log_pmsg_file_write) {
#ifdef __ANDROID__
#ifndef NO_PSTORE
  __android_log_close();
  if (getuid() == AID_ROOT) {
    tested__android_log_close = true;
    bool pmsgActiveAfter__android_log_close = isPmsgActive();
    bool logdwActiveAfter__android_log_close = isLogdwActive();
    EXPECT_FALSE(pmsgActiveAfter__android_log_close);
    EXPECT_FALSE(logdwActiveAfter__android_log_close);
  } else if (!tested__android_log_close) {
    fprintf(stderr, "WARNING: can not test __android_log_close()\n");
  }
  int return__android_log_pmsg_file_write = __android_log_pmsg_file_write(
      LOG_ID_CRASH, ANDROID_LOG_VERBOSE, __pmsg_file, max_payload_buf,
      sizeof(max_payload_buf));
  EXPECT_LT(0, return__android_log_pmsg_file_write);
  if (return__android_log_pmsg_file_write == -ENOMEM) {
    fprintf(stderr,
            "Kernel does not have space allocated to pmsg pstore driver "
            "configured\n");
  } else if (!return__android_log_pmsg_file_write) {
    fprintf(stderr,
            "Reboot, ensure file %s matches\n"
            "with liblog.__android_log_msg_file_read test\n",
            __pmsg_file);
  }
  bool pmsgActiveAfter__android_pmsg_file_write;
  bool logdwActiveAfter__android_pmsg_file_write;
  if (getuid() == AID_ROOT) {
    pmsgActiveAfter__android_pmsg_file_write = isPmsgActive();
    logdwActiveAfter__android_pmsg_file_write = isLogdwActive();
    EXPECT_FALSE(pmsgActiveAfter__android_pmsg_file_write);
    EXPECT_FALSE(logdwActiveAfter__android_pmsg_file_write);
  }
  EXPECT_LT(
      0, __android_log_buf_print(LOG_ID_MAIN, ANDROID_LOG_INFO,
                                 "TEST__android_log_pmsg_file_write", "main"));
  if (getuid() == AID_ROOT) {
    bool pmsgActiveAfter__android_log_buf_print = isPmsgActive();
    bool logdwActiveAfter__android_log_buf_print = isLogdwActive();
    EXPECT_TRUE(pmsgActiveAfter__android_log_buf_print);
    EXPECT_TRUE(logdwActiveAfter__android_log_buf_print);
  }
  EXPECT_LT(0, __android_log_pmsg_file_write(LOG_ID_CRASH, ANDROID_LOG_VERBOSE,
                                             __pmsg_file, max_payload_buf,
                                             sizeof(max_payload_buf)));
  if (getuid() == AID_ROOT) {
    pmsgActiveAfter__android_pmsg_file_write = isPmsgActive();
    logdwActiveAfter__android_pmsg_file_write = isLogdwActive();
    EXPECT_TRUE(pmsgActiveAfter__android_pmsg_file_write);
    EXPECT_TRUE(logdwActiveAfter__android_pmsg_file_write);
  }
#else  /* NO_PSTORE */
  GTEST_LOG_(INFO) << "This test does nothing because of NO_PSTORE.\n";
#endif /* NO_PSTORE */
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}

#ifdef __ANDROID__
#ifndef NO_PSTORE
static ssize_t __pmsg_fn(log_id_t logId, char prio, const char* filename,
                         const char* buf, size_t len, void* arg) {
  EXPECT_TRUE(NULL == arg);
  EXPECT_EQ(LOG_ID_CRASH, logId);
  EXPECT_EQ(ANDROID_LOG_VERBOSE, prio);
  EXPECT_FALSE(NULL == strstr(__pmsg_file, filename));
  EXPECT_EQ(len, sizeof(max_payload_buf));
  EXPECT_STREQ(max_payload_buf, buf);

  ++signaled;
  if ((len != sizeof(max_payload_buf)) || strcmp(max_payload_buf, buf)) {
    fprintf(stderr, "comparison fails on content \"%s\"\n", buf);
  }
  return arg || (LOG_ID_CRASH != logId) || (ANDROID_LOG_VERBOSE != prio) ||
                 !strstr(__pmsg_file, filename) ||
                 (len != sizeof(max_payload_buf)) ||
                 !!strcmp(max_payload_buf, buf)
             ? -ENOEXEC
             : 1;
}
#endif /* NO_PSTORE */
#endif

TEST(liblog, __android_log_pmsg_file_read) {
#ifdef __ANDROID__
#ifndef NO_PSTORE
  signaled = 0;

  __android_log_close();
  if (getuid() == AID_ROOT) {
    tested__android_log_close = true;
    bool pmsgActiveAfter__android_log_close = isPmsgActive();
    bool logdwActiveAfter__android_log_close = isLogdwActive();
    EXPECT_FALSE(pmsgActiveAfter__android_log_close);
    EXPECT_FALSE(logdwActiveAfter__android_log_close);
  } else if (!tested__android_log_close) {
    fprintf(stderr, "WARNING: can not test __android_log_close()\n");
  }

  ssize_t ret = __android_log_pmsg_file_read(LOG_ID_CRASH, ANDROID_LOG_VERBOSE,
                                             __pmsg_file, __pmsg_fn, NULL);

  if (getuid() == AID_ROOT) {
    bool pmsgActiveAfter__android_log_pmsg_file_read = isPmsgActive();
    bool logdwActiveAfter__android_log_pmsg_file_read = isLogdwActive();
    EXPECT_FALSE(pmsgActiveAfter__android_log_pmsg_file_read);
    EXPECT_FALSE(logdwActiveAfter__android_log_pmsg_file_read);
  }

  if (ret == -ENOENT) {
    fprintf(stderr,
            "No pre-boot results of liblog.__android_log_mesg_file_write to "
            "compare with,\n"
            "false positive test result.\n");
    return;
  }

  EXPECT_LT(0, ret);
  EXPECT_EQ(1U, signaled);
#else  /* NO_PSTORE */
  GTEST_LOG_(INFO) << "This test does nothing because of NO_PSTORE.\n";
#endif /* NO_PSTORE */
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
#endif  // ENABLE_FLAKY_TESTS
