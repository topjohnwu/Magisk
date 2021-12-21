/*
 * Copyright (C) 2013-2014 The Android Open Source Project
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

#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <unordered_set>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <benchmark/benchmark.h>
#include <cutils/sockets.h>
#include <log/event_tag_map.h>
#include <log/log_read.h>
#include <private/android_logger.h>

BENCHMARK_MAIN();

// enhanced version of LOG_FAILURE_RETRY to add support for EAGAIN and
// non-syscall libs. Since we are benchmarking, or using this in the emergency
// signal to stuff a terminating code, we do NOT want to introduce
// a syscall or usleep on EAGAIN retry.
#define LOG_FAILURE_RETRY(exp)                                           \
  ({                                                                     \
    typeof(exp) _rc;                                                     \
    do {                                                                 \
      _rc = (exp);                                                       \
    } while (((_rc == -1) && ((errno == EINTR) || (errno == EAGAIN))) || \
             (_rc == -EINTR) || (_rc == -EAGAIN));                       \
    _rc;                                                                 \
  })

/*
 *	Measure the fastest rate we can reliabley stuff print messages into
 * the log at high pressure. Expect this to be less than double the process
 * wakeup time (2ms?)
 */
static void BM_log_maximum_retry(benchmark::State& state) {
  while (state.KeepRunning()) {
    LOG_FAILURE_RETRY(__android_log_print(ANDROID_LOG_INFO, "BM_log_maximum_retry", "%" PRIu64,
                                          state.iterations()));
  }
}
BENCHMARK(BM_log_maximum_retry);

/*
 *	Measure the fastest rate we can stuff print messages into the log
 * at high pressure. Expect this to be less than double the process wakeup
 * time (2ms?)
 */
static void BM_log_maximum(benchmark::State& state) {
  while (state.KeepRunning()) {
    __android_log_print(ANDROID_LOG_INFO, "BM_log_maximum", "%" PRIu64, state.iterations());
  }
}
BENCHMARK(BM_log_maximum);

/*
 *	Measure the time it takes to collect the time using
 * discrete acquisition (state.PauseTiming() to state.ResumeTiming())
 * under light load. Expect this to be a syscall period (2us) or
 * data read time if zero-syscall.
 *
 * vdso support in the kernel and the library can allow
 * clock_gettime to be zero-syscall, but there there does remain some
 * benchmarking overhead to pause and resume; assumptions are both are
 * covered.
 */
static void BM_clock_overhead(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_clock_overhead);

static void do_clock_overhead(benchmark::State& state, clockid_t clk_id) {
  timespec t;
  while (state.KeepRunning()) {
    clock_gettime(clk_id, &t);
  }
}

static void BM_time_clock_gettime_REALTIME(benchmark::State& state) {
  do_clock_overhead(state, CLOCK_REALTIME);
}
BENCHMARK(BM_time_clock_gettime_REALTIME);

static void BM_time_clock_gettime_MONOTONIC(benchmark::State& state) {
  do_clock_overhead(state, CLOCK_MONOTONIC);
}
BENCHMARK(BM_time_clock_gettime_MONOTONIC);

static void BM_time_clock_gettime_MONOTONIC_syscall(benchmark::State& state) {
  timespec t;
  while (state.KeepRunning()) {
    syscall(__NR_clock_gettime, CLOCK_MONOTONIC, &t);
  }
}
BENCHMARK(BM_time_clock_gettime_MONOTONIC_syscall);

static void BM_time_clock_gettime_MONOTONIC_RAW(benchmark::State& state) {
  do_clock_overhead(state, CLOCK_MONOTONIC_RAW);
}
BENCHMARK(BM_time_clock_gettime_MONOTONIC_RAW);

static void BM_time_clock_gettime_BOOTTIME(benchmark::State& state) {
  do_clock_overhead(state, CLOCK_BOOTTIME);
}
BENCHMARK(BM_time_clock_gettime_BOOTTIME);

static void BM_time_clock_getres_MONOTONIC(benchmark::State& state) {
  timespec t;
  while (state.KeepRunning()) {
    clock_getres(CLOCK_MONOTONIC, &t);
  }
}
BENCHMARK(BM_time_clock_getres_MONOTONIC);

static void BM_time_clock_getres_MONOTONIC_syscall(benchmark::State& state) {
  timespec t;
  while (state.KeepRunning()) {
    syscall(__NR_clock_getres, CLOCK_MONOTONIC, &t);
  }
}
BENCHMARK(BM_time_clock_getres_MONOTONIC_syscall);

static void BM_time_time(benchmark::State& state) {
  while (state.KeepRunning()) {
    time_t now;
    now = time(&now);
  }
}
BENCHMARK(BM_time_time);

/*
 * Measure the time it takes to submit the android logging data to pstore
 */
static void BM_pmsg_short(benchmark::State& state) {
  int pstore_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
  if (pstore_fd < 0) {
    state.SkipWithError("/dev/pmsg0");
    return;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsg_header;
   *      // what we provide to socket
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  android_pmsg_log_header_t pmsg_header;
  pmsg_header.magic = LOGGER_MAGIC;
  pmsg_header.len =
      sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t);
  pmsg_header.uid = getuid();
  pmsg_header.pid = getpid();

  android_log_header_t header;
  header.tid = gettid();
  header.realtime.tv_sec = ts.tv_sec;
  header.realtime.tv_nsec = ts.tv_nsec;

  static const unsigned nr = 1;
  static const unsigned header_length = 2;
  struct iovec newVec[nr + header_length];

  newVec[0].iov_base = (unsigned char*)&pmsg_header;
  newVec[0].iov_len = sizeof(pmsg_header);
  newVec[1].iov_base = (unsigned char*)&header;
  newVec[1].iov_len = sizeof(header);

  android_log_event_int_t buffer;

  header.id = LOG_ID_EVENTS;
  buffer.header.tag = 0;
  buffer.payload.type = EVENT_TYPE_INT;
  uint32_t snapshot = 0;
  buffer.payload.data = snapshot;

  newVec[2].iov_base = &buffer;
  newVec[2].iov_len = sizeof(buffer);

  while (state.KeepRunning()) {
    ++snapshot;
    buffer.payload.data = snapshot;
    writev(pstore_fd, newVec, nr);
  }
  state.PauseTiming();
  close(pstore_fd);
}
BENCHMARK(BM_pmsg_short);

/*
 * Measure the time it takes to submit the android logging data to pstore
 * best case aligned single block.
 */
static void BM_pmsg_short_aligned(benchmark::State& state) {
  int pstore_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
  if (pstore_fd < 0) {
    state.SkipWithError("/dev/pmsg0");
    return;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsg_header;
   *      // what we provide to socket
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct packet {
    android_pmsg_log_header_t pmsg_header;
    android_log_header_t header;
    android_log_event_int_t payload;
  };
  alignas(8) char buf[sizeof(struct packet) + 8];
  memset(buf, 0, sizeof(buf));
  struct packet* buffer = (struct packet*)(((uintptr_t)buf + 7) & ~7);
  if (((uintptr_t)&buffer->pmsg_header) & 7) {
    fprintf(stderr, "&buffer=0x%p iterations=%" PRIu64 "\n", &buffer->pmsg_header,
            state.iterations());
  }

  buffer->pmsg_header.magic = LOGGER_MAGIC;
  buffer->pmsg_header.len =
      sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t);
  buffer->pmsg_header.uid = getuid();
  buffer->pmsg_header.pid = getpid();

  buffer->header.tid = gettid();
  buffer->header.realtime.tv_sec = ts.tv_sec;
  buffer->header.realtime.tv_nsec = ts.tv_nsec;

  buffer->header.id = LOG_ID_EVENTS;
  buffer->payload.header.tag = 0;
  buffer->payload.payload.type = EVENT_TYPE_INT;
  uint32_t snapshot = 0;
  buffer->payload.payload.data = snapshot;

  while (state.KeepRunning()) {
    ++snapshot;
    buffer->payload.payload.data = snapshot;
    write(pstore_fd, &buffer->pmsg_header,
          sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t) +
              sizeof(android_log_event_int_t));
  }
  state.PauseTiming();
  close(pstore_fd);
}
BENCHMARK(BM_pmsg_short_aligned);

/*
 * Measure the time it takes to submit the android logging data to pstore
 * best case aligned single block.
 */
static void BM_pmsg_short_unaligned1(benchmark::State& state) {
  int pstore_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
  if (pstore_fd < 0) {
    state.SkipWithError("/dev/pmsg0");
    return;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsg_header;
   *      // what we provide to socket
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct packet {
    android_pmsg_log_header_t pmsg_header;
    android_log_header_t header;
    android_log_event_int_t payload;
  };
  alignas(8) char buf[sizeof(struct packet) + 8];
  memset(buf, 0, sizeof(buf));
  struct packet* buffer = (struct packet*)((((uintptr_t)buf + 7) & ~7) + 1);
  if ((((uintptr_t)&buffer->pmsg_header) & 7) != 1) {
    fprintf(stderr, "&buffer=0x%p iterations=%" PRIu64 "\n", &buffer->pmsg_header,
            state.iterations());
  }

  buffer->pmsg_header.magic = LOGGER_MAGIC;
  buffer->pmsg_header.len =
      sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t);
  buffer->pmsg_header.uid = getuid();
  buffer->pmsg_header.pid = getpid();

  buffer->header.tid = gettid();
  buffer->header.realtime.tv_sec = ts.tv_sec;
  buffer->header.realtime.tv_nsec = ts.tv_nsec;

  buffer->header.id = LOG_ID_EVENTS;
  buffer->payload.header.tag = 0;
  buffer->payload.payload.type = EVENT_TYPE_INT;
  uint32_t snapshot = 0;
  buffer->payload.payload.data = snapshot;

  while (state.KeepRunning()) {
    ++snapshot;
    buffer->payload.payload.data = snapshot;
    write(pstore_fd, &buffer->pmsg_header,
          sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t) +
              sizeof(android_log_event_int_t));
  }
  state.PauseTiming();
  close(pstore_fd);
}
BENCHMARK(BM_pmsg_short_unaligned1);

/*
 * Measure the time it takes to submit the android logging data to pstore
 * best case aligned single block.
 */
static void BM_pmsg_long_aligned(benchmark::State& state) {
  int pstore_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
  if (pstore_fd < 0) {
    state.SkipWithError("/dev/pmsg0");
    return;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsg_header;
   *      // what we provide to socket
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct packet {
    android_pmsg_log_header_t pmsg_header;
    android_log_header_t header;
    android_log_event_int_t payload;
  };
  alignas(8) char buf[sizeof(struct packet) + 8 + LOGGER_ENTRY_MAX_PAYLOAD];
  memset(buf, 0, sizeof(buf));
  struct packet* buffer = (struct packet*)(((uintptr_t)buf + 7) & ~7);
  if (((uintptr_t)&buffer->pmsg_header) & 7) {
    fprintf(stderr, "&buffer=0x%p iterations=%" PRIu64 "\n", &buffer->pmsg_header,
            state.iterations());
  }

  buffer->pmsg_header.magic = LOGGER_MAGIC;
  buffer->pmsg_header.len =
      sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t);
  buffer->pmsg_header.uid = getuid();
  buffer->pmsg_header.pid = getpid();

  buffer->header.tid = gettid();
  buffer->header.realtime.tv_sec = ts.tv_sec;
  buffer->header.realtime.tv_nsec = ts.tv_nsec;

  buffer->header.id = LOG_ID_EVENTS;
  buffer->payload.header.tag = 0;
  buffer->payload.payload.type = EVENT_TYPE_INT;
  uint32_t snapshot = 0;
  buffer->payload.payload.data = snapshot;

  while (state.KeepRunning()) {
    ++snapshot;
    buffer->payload.payload.data = snapshot;
    write(pstore_fd, &buffer->pmsg_header, LOGGER_ENTRY_MAX_PAYLOAD);
  }
  state.PauseTiming();
  close(pstore_fd);
}
BENCHMARK(BM_pmsg_long_aligned);

/*
 * Measure the time it takes to submit the android logging data to pstore
 * best case aligned single block.
 */
static void BM_pmsg_long_unaligned1(benchmark::State& state) {
  int pstore_fd = TEMP_FAILURE_RETRY(open("/dev/pmsg0", O_WRONLY | O_CLOEXEC));
  if (pstore_fd < 0) {
    state.SkipWithError("/dev/pmsg0");
    return;
  }

  /*
   *  struct {
   *      // what we provide to pstore
   *      android_pmsg_log_header_t pmsg_header;
   *      // what we provide to socket
   *      android_log_header_t header;
   *      // caller provides
   *      union {
   *          struct {
   *              char     prio;
   *              char     payload[];
   *          } string;
   *          struct {
   *              uint32_t tag
   *              char     payload[];
   *          } binary;
   *      };
   *  };
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct packet {
    android_pmsg_log_header_t pmsg_header;
    android_log_header_t header;
    android_log_event_int_t payload;
  };
  alignas(8) char buf[sizeof(struct packet) + 8 + LOGGER_ENTRY_MAX_PAYLOAD];
  memset(buf, 0, sizeof(buf));
  struct packet* buffer = (struct packet*)((((uintptr_t)buf + 7) & ~7) + 1);
  if ((((uintptr_t)&buffer->pmsg_header) & 7) != 1) {
    fprintf(stderr, "&buffer=0x%p iterations=%" PRIu64 "\n", &buffer->pmsg_header,
            state.iterations());
  }

  buffer->pmsg_header.magic = LOGGER_MAGIC;
  buffer->pmsg_header.len =
      sizeof(android_pmsg_log_header_t) + sizeof(android_log_header_t);
  buffer->pmsg_header.uid = getuid();
  buffer->pmsg_header.pid = getpid();

  buffer->header.tid = gettid();
  buffer->header.realtime.tv_sec = ts.tv_sec;
  buffer->header.realtime.tv_nsec = ts.tv_nsec;

  buffer->header.id = LOG_ID_EVENTS;
  buffer->payload.header.tag = 0;
  buffer->payload.payload.type = EVENT_TYPE_INT;
  uint32_t snapshot = 0;
  buffer->payload.payload.data = snapshot;

  while (state.KeepRunning()) {
    ++snapshot;
    buffer->payload.payload.data = snapshot;
    write(pstore_fd, &buffer->pmsg_header, LOGGER_ENTRY_MAX_PAYLOAD);
  }
  state.PauseTiming();
  close(pstore_fd);
}
BENCHMARK(BM_pmsg_long_unaligned1);

/*
 *	Measure the time it takes to form sprintf plus time using
 * discrete acquisition under light load. Expect this to be a syscall period
 * (2us) or sprintf time if zero-syscall time.
 */
/* helper function */
static void test_print(const char* fmt, ...) {
  va_list ap;
  char buf[1024];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
}

#define logd_yield() sched_yield()  // allow logd to catch up
#define logd_sleep() usleep(50)     // really allow logd to catch up

/* performance test */
static void BM_sprintf_overhead(benchmark::State& state) {
  while (state.KeepRunning()) {
    test_print("BM_sprintf_overhead:%" PRIu64, state.iterations());
    state.PauseTiming();
    logd_yield();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_sprintf_overhead);

/*
 *	Measure the time it takes to submit the android printing logging call
 * using discrete acquisition discrete acquisition under light load. Expect
 * this to be a dozen or so syscall periods (40us) plus time to run *printf
 */
static void BM_log_print_overhead(benchmark::State& state) {
  while (state.KeepRunning()) {
    __android_log_print(ANDROID_LOG_INFO, "BM_log_overhead", "%" PRIu64, state.iterations());
    state.PauseTiming();
    logd_yield();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_log_print_overhead);

/*
 *	Measure the time it takes to submit the android event logging call
 * using discrete acquisition under light load. Expect this to be a long path
 * to logger to convert the unknown tag (0) into a tagname (less than 200us).
 */
static void BM_log_event_overhead(benchmark::State& state) {
  for (int64_t i = 0; state.KeepRunning(); ++i) {
    // log tag number 0 is not known, nor shall it ever be known
    __android_log_btwrite(0, EVENT_TYPE_LONG, &i, sizeof(i));
    state.PauseTiming();
    logd_yield();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_log_event_overhead);

/*
 *	Measure the time it takes to submit the android event logging call
 * using discrete acquisition under light load with a known logtag.  Expect
 * this to be a dozen or so syscall periods (less than 40us)
 */
static void BM_log_event_overhead_42(benchmark::State& state) {
  for (int64_t i = 0; state.KeepRunning(); ++i) {
    // In system/core/logcat/event.logtags:
    // # These are used for testing, do not modify without updating
    // # tests/framework-tests/src/android/util/EventLogFunctionalTest.java.
    // # system/logging/liblog/tests/liblog_benchmark.cpp
    // # system/logging/liblog/tests/liblog_test.cpp
    // 42    answer (to life the universe etc|3)
    __android_log_btwrite(42, EVENT_TYPE_LONG, &i, sizeof(i));
    state.PauseTiming();
    logd_yield();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_log_event_overhead_42);

/*
 *	Measure the time it takes to submit the android event logging call
 * using discrete acquisition under very-light load (<1% CPU utilization).
 */
static void BM_log_light_overhead(benchmark::State& state) {
  for (int64_t i = 0; state.KeepRunning(); ++i) {
    __android_log_btwrite(0, EVENT_TYPE_LONG, &i, sizeof(i));
    state.PauseTiming();
    usleep(10000);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_log_light_overhead);

static void caught_latency(int /*signum*/) {
  unsigned long long v = 0xDEADBEEFA55A5AA5ULL;

  LOG_FAILURE_RETRY(__android_log_btwrite(0, EVENT_TYPE_LONG, &v, sizeof(v)));
}

static unsigned long long caught_convert(char* cp) {
  unsigned long long l = cp[0] & 0xFF;
  l |= (unsigned long long)(cp[1] & 0xFF) << 8;
  l |= (unsigned long long)(cp[2] & 0xFF) << 16;
  l |= (unsigned long long)(cp[3] & 0xFF) << 24;
  l |= (unsigned long long)(cp[4] & 0xFF) << 32;
  l |= (unsigned long long)(cp[5] & 0xFF) << 40;
  l |= (unsigned long long)(cp[6] & 0xFF) << 48;
  l |= (unsigned long long)(cp[7] & 0xFF) << 56;
  return l;
}

static const int alarm_time = 3;

/*
 *	Measure the time it takes for the logd posting call to acquire the
 * timestamp to place into the internal record.  Expect this to be less than
 * 4 syscalls (3us).  This test uses manual injection of timing because it is
 * comparing the timestamp at send, and then picking up the corresponding log
 * end-to-end long path from logd to see what actual timestamp was submitted.
 */
static void BM_log_latency(benchmark::State& state) {
  pid_t pid = getpid();

  struct logger_list* logger_list = android_logger_list_open(LOG_ID_EVENTS, 0, 0, pid);

  if (!logger_list) {
    fprintf(stderr, "Unable to open events log: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  signal(SIGALRM, caught_latency);
  alarm(alarm_time);

  for (size_t j = 0; state.KeepRunning() && j < 10 * state.iterations(); ++j) {
  retry:  // We allow transitory errors (logd overloaded) to be retried.
    log_time ts;
    LOG_FAILURE_RETRY((ts = log_time(CLOCK_REALTIME),
                       android_btWriteLog(0, EVENT_TYPE_LONG, &ts, sizeof(ts))));

    for (;;) {
      log_msg log_msg;
      int ret = android_logger_list_read(logger_list, &log_msg);
      alarm(alarm_time);

      if (ret <= 0) {
        state.SkipWithError("android_logger_list_read");
        break;
      }
      if ((log_msg.entry.len != (4 + 1 + 8)) ||
          (log_msg.id() != LOG_ID_EVENTS)) {
        continue;
      }

      char* eventData = log_msg.msg();

      if (!eventData || (eventData[4] != EVENT_TYPE_LONG)) {
        continue;
      }
      log_time* tx = reinterpret_cast<log_time*>(eventData + 4 + 1);
      if (ts != *tx) {
        if (0xDEADBEEFA55A5AA5ULL == caught_convert(eventData + 4 + 1)) {
          state.SkipWithError("signal");
          break;
        }
        continue;
      }

      uint64_t start = ts.nsec();
      uint64_t end = log_msg.nsec();
      if (end < start) goto retry;
      state.SetIterationTime((end - start) / (double)NS_PER_SEC);
      break;
    }
  }

  signal(SIGALRM, SIG_DFL);
  alarm(0);

  android_logger_list_free(logger_list);
}
// Default gets out of hand for this test, so we set a reasonable number of
// iterations for a timely result.
BENCHMARK(BM_log_latency)->UseManualTime()->Iterations(200);

static void caught_delay(int /*signum*/) {
  unsigned long long v = 0xDEADBEEFA55A5AA6ULL;

  LOG_FAILURE_RETRY(__android_log_btwrite(0, EVENT_TYPE_LONG, &v, sizeof(v)));
}

/*
 *	Measure the time it takes for the logd posting call to make it into
 * the logs. Expect this to be less than double the process wakeup time (2ms).
 */
static void BM_log_delay(benchmark::State& state) {
  pid_t pid = getpid();

  struct logger_list* logger_list = android_logger_list_open(LOG_ID_EVENTS, 0, 0, pid);

  if (!logger_list) {
    fprintf(stderr, "Unable to open events log: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  signal(SIGALRM, caught_delay);
  alarm(alarm_time);

  while (state.KeepRunning()) {
    log_time ts(CLOCK_REALTIME);

    LOG_FAILURE_RETRY(android_btWriteLog(0, EVENT_TYPE_LONG, &ts, sizeof(ts)));

    for (;;) {
      log_msg log_msg;
      int ret = android_logger_list_read(logger_list, &log_msg);
      alarm(alarm_time);

      if (ret <= 0) {
        state.SkipWithError("android_logger_list_read");
        break;
      }
      if ((log_msg.entry.len != (4 + 1 + 8)) ||
          (log_msg.id() != LOG_ID_EVENTS)) {
        continue;
      }

      char* eventData = log_msg.msg();

      if (!eventData || (eventData[4] != EVENT_TYPE_LONG)) {
        continue;
      }
      log_time* tx = reinterpret_cast<log_time*>(eventData + 4 + 1);
      if (ts != *tx) {
        if (0xDEADBEEFA55A5AA6ULL == caught_convert(eventData + 4 + 1)) {
          state.SkipWithError("signal");
          break;
        }
        continue;
      }

      break;
    }
  }
  state.PauseTiming();

  signal(SIGALRM, SIG_DFL);
  alarm(0);

  android_logger_list_free(logger_list);
}
BENCHMARK(BM_log_delay);

/*
 *	Measure the time it takes for __android_log_is_loggable.
 */
static void BM_is_loggable(benchmark::State& state) {
  static const char logd[] = "logd";

  while (state.KeepRunning()) {
    __android_log_is_loggable_len(ANDROID_LOG_WARN, logd, strlen(logd),
                                  ANDROID_LOG_VERBOSE);
  }
}
BENCHMARK(BM_is_loggable);

/*
 *	Measure the time it takes for __android_log_security.
 */
static void BM_security(benchmark::State& state) {
  while (state.KeepRunning()) {
    __android_log_security();
  }
}
BENCHMARK(BM_security);

// Keep maps around for multiple iterations
static std::unordered_set<uint32_t> set;
static EventTagMap* map;

static bool prechargeEventMap() {
  if (map) return true;

  fprintf(stderr, "Precharge: start\n");

  map = android_openEventTagMap(NULL);
  for (uint32_t tag = 1; tag < USHRT_MAX; ++tag) {
    size_t len;
    if (android_lookupEventTag_len(map, &len, tag) == NULL) continue;
    set.insert(tag);
  }

  fprintf(stderr, "Precharge: stop %zu\n", set.size());

  return true;
}

/*
 *	Measure the time it takes for android_lookupEventTag_len
 */
static void BM_lookupEventTag(benchmark::State& state) {
  prechargeEventMap();

  std::unordered_set<uint32_t>::const_iterator it = set.begin();

  while (state.KeepRunning()) {
    size_t len;
    android_lookupEventTag_len(map, &len, (*it));
    ++it;
    if (it == set.end()) it = set.begin();
  }
}
BENCHMARK(BM_lookupEventTag);

/*
 *	Measure the time it takes for android_lookupEventTag_len
 */
static uint32_t notTag = 1;

static void BM_lookupEventTag_NOT(benchmark::State& state) {
  prechargeEventMap();

  while (set.find(notTag) != set.end()) {
    ++notTag;
    if (notTag >= USHRT_MAX) notTag = 1;
  }

  while (state.KeepRunning()) {
    size_t len;
    android_lookupEventTag_len(map, &len, notTag);
  }

  ++notTag;
  if (notTag >= USHRT_MAX) notTag = 1;
}
BENCHMARK(BM_lookupEventTag_NOT);

/*
 *	Measure the time it takes for android_lookupEventFormat_len
 */
static void BM_lookupEventFormat(benchmark::State& state) {
  prechargeEventMap();

  std::unordered_set<uint32_t>::const_iterator it = set.begin();

  while (state.KeepRunning()) {
    size_t len;
    android_lookupEventFormat_len(map, &len, (*it));
    ++it;
    if (it == set.end()) it = set.begin();
  }
}
BENCHMARK(BM_lookupEventFormat);

// Must be functionally identical to liblog internal SendLogdControlMessage()
static void send_to_control(char* buf, size_t len) {
  int sock =
      socket_local_client("logd", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM | SOCK_CLOEXEC);
  if (sock < 0) return;
  size_t writeLen = strlen(buf) + 1;

  ssize_t ret = TEMP_FAILURE_RETRY(write(sock, buf, writeLen));
  if (ret <= 0) {
    close(sock);
    return;
  }
  while ((ret = read(sock, buf, len)) > 0) {
    if (((size_t)ret == len) || (len < PAGE_SIZE)) {
      break;
    }
    len -= ret;
    buf += ret;

    struct pollfd p = {.fd = sock, .events = POLLIN, .revents = 0 };

    ret = poll(&p, 1, 20);
    if ((ret <= 0) || !(p.revents & POLLIN)) {
      break;
    }
  }
  close(sock);
}

static void BM_lookupEventTagNum_logd_new(benchmark::State& state) {
  fprintf(stderr,
          "WARNING: "
          "This test can cause logd to grow in size and hit DOS limiter\n");
  // Make copies
  static const char empty_event_log_tags[] = "# content owned by logd\n";
  static const char dev_event_log_tags_path[] = "/dev/event-log-tags";
  std::string dev_event_log_tags;
  if (android::base::ReadFileToString(dev_event_log_tags_path,
                                      &dev_event_log_tags) &&
      (dev_event_log_tags.length() == 0)) {
    dev_event_log_tags = empty_event_log_tags;
  }
  static const char data_event_log_tags_path[] =
      "/data/misc/logd/event-log-tags";
  std::string data_event_log_tags;
  if (android::base::ReadFileToString(data_event_log_tags_path,
                                      &data_event_log_tags) &&
      (data_event_log_tags.length() == 0)) {
    data_event_log_tags = empty_event_log_tags;
  }

  while (state.KeepRunning()) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    log_time now(CLOCK_MONOTONIC);
    char name[64];
    snprintf(name, sizeof(name), "a%" PRIu64, now.nsec());
    snprintf(buffer, sizeof(buffer), "getEventTag name=%s format=\"(new|1)\"",
             name);
    state.ResumeTiming();
    send_to_control(buffer, sizeof(buffer));
    state.PauseTiming();
  }

  // Restore copies (logd still know about them, until crash or reboot)
  if (dev_event_log_tags.length() &&
      !android::base::WriteStringToFile(dev_event_log_tags,
                                        dev_event_log_tags_path)) {
    fprintf(stderr,
            "WARNING: "
            "failed to restore %s\n",
            dev_event_log_tags_path);
  }
  if (data_event_log_tags.length() &&
      !android::base::WriteStringToFile(data_event_log_tags,
                                        data_event_log_tags_path)) {
    fprintf(stderr,
            "WARNING: "
            "failed to restore %s\n",
            data_event_log_tags_path);
  }
  fprintf(stderr,
          "WARNING: "
          "Restarting logd to make it forget what we just did\n");
  system("stop logd ; start logd");
}
BENCHMARK(BM_lookupEventTagNum_logd_new);

static void BM_lookupEventTagNum_logd_existing(benchmark::State& state) {
  prechargeEventMap();

  std::unordered_set<uint32_t>::const_iterator it = set.begin();

  while (state.KeepRunning()) {
    size_t len;
    const char* name = android_lookupEventTag_len(map, &len, (*it));
    std::string Name(name, len);
    const char* format = android_lookupEventFormat_len(map, &len, (*it));
    std::string Format(format, len);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "getEventTag name=%s format=\"%s\"",
             Name.c_str(), Format.c_str());

    state.ResumeTiming();
    send_to_control(buffer, sizeof(buffer));
    state.PauseTiming();
    ++it;
    if (it == set.end()) it = set.begin();
  }
}
BENCHMARK(BM_lookupEventTagNum_logd_existing);

static void BM_log_verbose_overhead(benchmark::State& state) {
  std::string test_log_tag = "liblog_verbose_tag";
  android::base::SetProperty("log.tag." + test_log_tag, "I");
  for (auto _ : state) {
    __android_log_print(ANDROID_LOG_VERBOSE, test_log_tag.c_str(), "%s test log message %d %d",
                        "test test", 123, 456);
  }
  android::base::SetProperty("log.tag." + test_log_tag, "");
}
BENCHMARK(BM_log_verbose_overhead);
