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

#pragma once

//
// Google-style C++ logging.
//

// This header provides a C++ stream interface to logging.
//
// To log:
//
//   LOG(INFO) << "Some text; " << some_value;
//
// Replace `INFO` with any severity from `enum LogSeverity`.
// Most devices filter out VERBOSE logs by default, run
// `adb shell setprop log.tag.<TAG> V` to see them in adb logcat.
//
// To log the result of a failed function and include the string
// representation of `errno` at the end:
//
//   PLOG(ERROR) << "Write failed";
//
// The output will be something like `Write failed: I/O error`.
// Remember this as 'P' as in perror(3).
//
// To output your own types, simply implement operator<< as normal.
//
// By default, output goes to logcat on Android and stderr on the host.
// A process can use `SetLogger` to decide where all logging goes.
// Implementations are provided for logcat, stderr, and dmesg.
//
// By default, the process' name is used as the log tag.
// Code can choose a specific log tag by defining LOG_TAG
// before including this header.

// This header also provides assertions:
//
//   CHECK(must_be_true);
//   CHECK_EQ(a, b) << z_is_interesting_too;

// NOTE: For Windows, you must include logging.h after windows.h to allow the
// following code to suppress the evil ERROR macro:
#ifdef _WIN32
// windows.h includes wingdi.h which defines an evil macro ERROR.
#ifdef ERROR
#undef ERROR
#endif
#endif

#include <functional>
#include <memory>
#include <ostream>

#include "android-base/errno_restorer.h"
#include "android-base/macros.h"

// Note: DO NOT USE DIRECTLY. Use LOG_TAG instead.
#ifdef _LOG_TAG_INTERNAL
#error "_LOG_TAG_INTERNAL must not be defined"
#endif
#ifdef LOG_TAG
#define _LOG_TAG_INTERNAL LOG_TAG
#else
#define _LOG_TAG_INTERNAL nullptr
#endif

namespace android {
namespace base {

enum LogSeverity {
  VERBOSE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL_WITHOUT_ABORT,  // For loggability tests, this is considered identical to FATAL.
  FATAL,
};

enum LogId {
  DEFAULT,
  MAIN,
  SYSTEM,
  RADIO,
  CRASH,
};

using LogFunction = std::function<void(LogId /*log_buffer_id*/,
                                       LogSeverity /*severity*/,
                                       const char* /*tag*/,
                                       const char* /*file*/,
                                       unsigned int /*line*/,
                                       const char* /*message*/)>;
using AbortFunction = std::function<void(const char* /*abort_message*/)>;

// Loggers for use with InitLogging/SetLogger.

// Log to the kernel log (dmesg).
void KernelLogger(LogId log_buffer_id, LogSeverity severity, const char* tag, const char* file, unsigned int line, const char* message);
// Log to stderr in the full logcat format (with pid/tid/time/tag details).
void StderrLogger(LogId log_buffer_id, LogSeverity severity, const char* tag, const char* file, unsigned int line, const char* message);
// Log just the message to stdout/stderr (without pid/tid/time/tag details).
// The choice of stdout versus stderr is based on the severity.
// Errors are also prefixed by the program name (as with err(3)/error(3)).
// Useful for replacing printf(3)/perror(3)/err(3)/error(3) in command-line tools.
void StdioLogger(LogId log_buffer_id, LogSeverity severity, const char* tag, const char* file, unsigned int line, const char* message);

void DefaultAborter(const char* abort_message);

void SetDefaultTag(const std::string& tag);

// The LogdLogger sends chunks of up to ~4000 bytes at a time to logd.  It does not prevent other
// threads from writing to logd between sending each chunk, so other threads may interleave their
// messages.  If preventing interleaving is required, then a custom logger that takes a lock before
// calling this logger should be provided.
class LogdLogger {
 public:
  explicit LogdLogger(LogId default_log_id = android::base::MAIN);

  void operator()(LogId, LogSeverity, const char* tag, const char* file,
                  unsigned int line, const char* message);

 private:
  LogId default_log_id_;
};

// Configure logging based on ANDROID_LOG_TAGS environment variable.
// We need to parse a string that looks like
//
//      *:v jdwp:d dalvikvm:d dalvikvm-gc:i dalvikvmi:i
//
// The tag (or '*' for the global level) comes first, followed by a colon and a
// letter indicating the minimum priority level we're expected to log.  This can
// be used to reveal or conceal logs with specific tags.
#ifdef __ANDROID__
#define INIT_LOGGING_DEFAULT_LOGGER LogdLogger()
#else
#define INIT_LOGGING_DEFAULT_LOGGER StderrLogger
#endif
void InitLogging(char* argv[],
                 LogFunction&& logger = INIT_LOGGING_DEFAULT_LOGGER,
                 AbortFunction&& aborter = DefaultAborter);
#undef INIT_LOGGING_DEFAULT_LOGGER

// Replace the current logger and return the old one.
LogFunction SetLogger(LogFunction&& logger);

// Replace the current aborter and return the old one.
AbortFunction SetAborter(AbortFunction&& aborter);

// A helper macro that produces an expression that accepts both a qualified name and an
// unqualified name for a LogSeverity, and returns a LogSeverity value.
// Note: DO NOT USE DIRECTLY. This is an implementation detail.
#define SEVERITY_LAMBDA(severity) ([&]() {    \
  using ::android::base::VERBOSE;             \
  using ::android::base::DEBUG;               \
  using ::android::base::INFO;                \
  using ::android::base::WARNING;             \
  using ::android::base::ERROR;               \
  using ::android::base::FATAL_WITHOUT_ABORT; \
  using ::android::base::FATAL;               \
  return (severity); }())

#ifdef __clang_analyzer__
// Clang's static analyzer does not see the conditional statement inside
// LogMessage's destructor that will abort on FATAL severity.
#define ABORT_AFTER_LOG_FATAL for (;; abort())

struct LogAbortAfterFullExpr {
  ~LogAbortAfterFullExpr() __attribute__((noreturn)) { abort(); }
  explicit operator bool() const { return false; }
};
// Provides an expression that evaluates to the truthiness of `x`, automatically
// aborting if `c` is true.
#define ABORT_AFTER_LOG_EXPR_IF(c, x) (((c) && ::android::base::LogAbortAfterFullExpr()) || (x))
// Note to the static analyzer that we always execute FATAL logs in practice.
#define MUST_LOG_MESSAGE(severity) (SEVERITY_LAMBDA(severity) == ::android::base::FATAL)
#else
#define ABORT_AFTER_LOG_FATAL
#define ABORT_AFTER_LOG_EXPR_IF(c, x) (x)
#define MUST_LOG_MESSAGE(severity) false
#endif
#define ABORT_AFTER_LOG_FATAL_EXPR(x) ABORT_AFTER_LOG_EXPR_IF(true, x)

// Defines whether the given severity will be logged or silently swallowed.
#define WOULD_LOG(severity)                                                              \
  (UNLIKELY(::android::base::ShouldLog(SEVERITY_LAMBDA(severity), _LOG_TAG_INTERNAL)) || \
   MUST_LOG_MESSAGE(severity))

// Get an ostream that can be used for logging at the given severity and to the default
// destination.
//
// Notes:
// 1) This will not check whether the severity is high enough. One should use WOULD_LOG to filter
//    usage manually.
// 2) This does not save and restore errno.
#define LOG_STREAM(severity)                                                                    \
  ::android::base::LogMessage(__FILE__, __LINE__, SEVERITY_LAMBDA(severity), _LOG_TAG_INTERNAL, \
                              -1)                                                               \
      .stream()

// Logs a message to logcat on Android otherwise to stderr. If the severity is
// FATAL it also causes an abort. For example:
//
//     LOG(FATAL) << "We didn't expect to reach here";
#define LOG(severity) LOGGING_PREAMBLE(severity) && LOG_STREAM(severity)

// Checks if we want to log something, and sets up appropriate RAII objects if
// so.
// Note: DO NOT USE DIRECTLY. This is an implementation detail.
#define LOGGING_PREAMBLE(severity)                                                         \
  (WOULD_LOG(severity) &&                                                                  \
   ABORT_AFTER_LOG_EXPR_IF((SEVERITY_LAMBDA(severity)) == ::android::base::FATAL, true) && \
   ::android::base::ErrnoRestorer())

// A variant of LOG that also logs the current errno value. To be used when
// library calls fail.
#define PLOG(severity)                                                           \
  LOGGING_PREAMBLE(severity) &&                                                  \
      ::android::base::LogMessage(__FILE__, __LINE__, SEVERITY_LAMBDA(severity), \
                                  _LOG_TAG_INTERNAL, errno)                      \
          .stream()

// Marker that code is yet to be implemented.
#define UNIMPLEMENTED(level) \
  LOG(level) << __PRETTY_FUNCTION__ << " unimplemented "

// Check whether condition x holds and LOG(FATAL) if not. The value of the
// expression x is only evaluated once. Extra logging can be appended using <<
// after. For example:
//
//     CHECK(false == true) results in a log message of
//       "Check failed: false == true".
#define CHECK(x)                                                                                 \
  LIKELY((x)) || ABORT_AFTER_LOG_FATAL_EXPR(false) ||                                            \
      ::android::base::LogMessage(__FILE__, __LINE__, ::android::base::FATAL, _LOG_TAG_INTERNAL, \
                                  -1)                                                            \
              .stream()                                                                          \
          << "Check failed: " #x << " "

// clang-format off
// Helper for CHECK_xx(x,y) macros.
#define CHECK_OP(LHS, RHS, OP)                                                                   \
  for (auto _values = ::android::base::MakeEagerEvaluator(LHS, RHS);                             \
       UNLIKELY(!(_values.lhs.v OP _values.rhs.v));                                              \
       /* empty */)                                                                              \
  ABORT_AFTER_LOG_FATAL                                                                          \
  ::android::base::LogMessage(__FILE__, __LINE__, ::android::base::FATAL, _LOG_TAG_INTERNAL, -1) \
          .stream()                                                                              \
      << "Check failed: " << #LHS << " " << #OP << " " << #RHS << " (" #LHS "="                  \
      << ::android::base::LogNullGuard<decltype(_values.lhs.v)>::Guard(_values.lhs.v)            \
      << ", " #RHS "="                                                                           \
      << ::android::base::LogNullGuard<decltype(_values.rhs.v)>::Guard(_values.rhs.v)            \
      << ") "
// clang-format on

// Check whether a condition holds between x and y, LOG(FATAL) if not. The value
// of the expressions x and y is evaluated once. Extra logging can be appended
// using << after. For example:
//
//     CHECK_NE(0 == 1, false) results in
//       "Check failed: false != false (0==1=false, false=false) ".
#define CHECK_EQ(x, y) CHECK_OP(x, y, == )
#define CHECK_NE(x, y) CHECK_OP(x, y, != )
#define CHECK_LE(x, y) CHECK_OP(x, y, <= )
#define CHECK_LT(x, y) CHECK_OP(x, y, < )
#define CHECK_GE(x, y) CHECK_OP(x, y, >= )
#define CHECK_GT(x, y) CHECK_OP(x, y, > )

// clang-format off
// Helper for CHECK_STRxx(s1,s2) macros.
#define CHECK_STROP(s1, s2, sense)                                             \
  while (UNLIKELY((strcmp(s1, s2) == 0) != (sense)))                           \
    ABORT_AFTER_LOG_FATAL                                                      \
    ::android::base::LogMessage(__FILE__, __LINE__,  ::android::base::FATAL,   \
                                 _LOG_TAG_INTERNAL, -1)                        \
        .stream()                                                              \
        << "Check failed: " << "\"" << (s1) << "\""                            \
        << ((sense) ? " == " : " != ") << "\"" << (s2) << "\""
// clang-format on

// Check for string (const char*) equality between s1 and s2, LOG(FATAL) if not.
#define CHECK_STREQ(s1, s2) CHECK_STROP(s1, s2, true)
#define CHECK_STRNE(s1, s2) CHECK_STROP(s1, s2, false)

// Perform the pthread function call(args), LOG(FATAL) on error.
#define CHECK_PTHREAD_CALL(call, args, what)                           \
  do {                                                                 \
    int rc = call args;                                                \
    if (rc != 0) {                                                     \
      errno = rc;                                                      \
      ABORT_AFTER_LOG_FATAL                                            \
      PLOG(FATAL) << #call << " failed for " << (what);                \
    }                                                                  \
  } while (false)

// DCHECKs are debug variants of CHECKs only enabled in debug builds. Generally
// CHECK should be used unless profiling identifies a CHECK as being in
// performance critical code.
#if defined(NDEBUG) && !defined(__clang_analyzer__)
static constexpr bool kEnableDChecks = false;
#else
static constexpr bool kEnableDChecks = true;
#endif

#define DCHECK(x) \
  if (::android::base::kEnableDChecks) CHECK(x)
#define DCHECK_EQ(x, y) \
  if (::android::base::kEnableDChecks) CHECK_EQ(x, y)
#define DCHECK_NE(x, y) \
  if (::android::base::kEnableDChecks) CHECK_NE(x, y)
#define DCHECK_LE(x, y) \
  if (::android::base::kEnableDChecks) CHECK_LE(x, y)
#define DCHECK_LT(x, y) \
  if (::android::base::kEnableDChecks) CHECK_LT(x, y)
#define DCHECK_GE(x, y) \
  if (::android::base::kEnableDChecks) CHECK_GE(x, y)
#define DCHECK_GT(x, y) \
  if (::android::base::kEnableDChecks) CHECK_GT(x, y)
#define DCHECK_STREQ(s1, s2) \
  if (::android::base::kEnableDChecks) CHECK_STREQ(s1, s2)
#define DCHECK_STRNE(s1, s2) \
  if (::android::base::kEnableDChecks) CHECK_STRNE(s1, s2)

namespace log_detail {

// Temporary storage for a single eagerly evaluated check expression operand.
template <typename T> struct Storage {
  template <typename U> explicit constexpr Storage(U&& u) : v(std::forward<U>(u)) {}
  explicit Storage(const Storage& t) = delete;
  explicit Storage(Storage&& t) = delete;
  T v;
};

// Partial specialization for smart pointers to avoid copying.
template <typename T> struct Storage<std::unique_ptr<T>> {
  explicit constexpr Storage(const std::unique_ptr<T>& ptr) : v(ptr.get()) {}
  const T* v;
};
template <typename T> struct Storage<std::shared_ptr<T>> {
  explicit constexpr Storage(const std::shared_ptr<T>& ptr) : v(ptr.get()) {}
  const T* v;
};

// Type trait that checks if a type is a (potentially const) char pointer.
template <typename T> struct IsCharPointer {
  using Pointee = std::remove_cv_t<std::remove_pointer_t<T>>;
  static constexpr bool value = std::is_pointer_v<T> &&
      (std::is_same_v<Pointee, char> || std::is_same_v<Pointee, signed char> ||
       std::is_same_v<Pointee, unsigned char>);
};

// Counterpart to Storage that depends on both operands. This is used to prevent
// char pointers being treated as strings in the log output - they might point
// to buffers of unprintable binary data.
template <typename LHS, typename RHS> struct StorageTypes {
  static constexpr bool voidptr = IsCharPointer<LHS>::value && IsCharPointer<RHS>::value;
  using LHSType = std::conditional_t<voidptr, const void*, LHS>;
  using RHSType = std::conditional_t<voidptr, const void*, RHS>;
};

// Temporary class created to evaluate the LHS and RHS, used with
// MakeEagerEvaluator to infer the types of LHS and RHS.
template <typename LHS, typename RHS>
struct EagerEvaluator {
  template <typename A, typename B> constexpr EagerEvaluator(A&& l, B&& r)
      : lhs(std::forward<A>(l)), rhs(std::forward<B>(r)) {}
  const Storage<typename StorageTypes<LHS, RHS>::LHSType> lhs;
  const Storage<typename StorageTypes<LHS, RHS>::RHSType> rhs;
};

}  // namespace log_detail

// Converts std::nullptr_t and null char pointers to the string "null"
// when writing the failure message.
template <typename T> struct LogNullGuard {
  static const T& Guard(const T& v) { return v; }
};
template <> struct LogNullGuard<std::nullptr_t> {
  static const char* Guard(const std::nullptr_t&) { return "(null)"; }
};
template <> struct LogNullGuard<char*> {
  static const char* Guard(const char* v) { return v ? v : "(null)"; }
};
template <> struct LogNullGuard<const char*> {
  static const char* Guard(const char* v) { return v ? v : "(null)"; }
};

// Helper function for CHECK_xx.
template <typename LHS, typename RHS>
constexpr auto MakeEagerEvaluator(LHS&& lhs, RHS&& rhs) {
  return log_detail::EagerEvaluator<std::decay_t<LHS>, std::decay_t<RHS>>(
      std::forward<LHS>(lhs), std::forward<RHS>(rhs));
}

// Data for the log message, not stored in LogMessage to avoid increasing the
// stack size.
class LogMessageData;

// A LogMessage is a temporarily scoped object used by LOG and the unlikely part
// of a CHECK. The destructor will abort if the severity is FATAL.
class LogMessage {
 public:
  // LogId has been deprecated, but this constructor must exist for prebuilts.
  LogMessage(const char* file, unsigned int line, LogId, LogSeverity severity, const char* tag,
             int error);
  LogMessage(const char* file, unsigned int line, LogSeverity severity, const char* tag, int error);

  ~LogMessage();

  // Returns the stream associated with the message, the LogMessage performs
  // output when it goes out of scope.
  std::ostream& stream();

  // The routine that performs the actual logging.
  static void LogLine(const char* file, unsigned int line, LogSeverity severity, const char* tag,
                      const char* msg);

 private:
  const std::unique_ptr<LogMessageData> data_;

  DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

// Get the minimum severity level for logging.
LogSeverity GetMinimumLogSeverity();

// Set the minimum severity level for logging, returning the old severity.
LogSeverity SetMinimumLogSeverity(LogSeverity new_severity);

// Return whether or not a log message with the associated tag should be logged.
bool ShouldLog(LogSeverity severity, const char* tag);

// Allows to temporarily change the minimum severity level for logging.
class ScopedLogSeverity {
 public:
  explicit ScopedLogSeverity(LogSeverity level);
  ~ScopedLogSeverity();

 private:
  LogSeverity old_;
};

}  // namespace base
}  // namespace android

namespace std {  // NOLINT(cert-dcl58-cpp)

// Emit a warning of ostream<< with std::string*. The intention was most likely to print *string.
//
// Note: for this to work, we need to have this in a namespace.
// Note: using a pragma because "-Wgcc-compat" (included in "-Weverything") complains about
//       diagnose_if.
// Note: to print the pointer, use "<< static_cast<const void*>(string_pointer)" instead.
// Note: a not-recommended alternative is to let Clang ignore the warning by adding
//       -Wno-user-defined-warnings to CPPFLAGS.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgcc-compat"
#define OSTREAM_STRING_POINTER_USAGE_WARNING \
    __attribute__((diagnose_if(true, "Unexpected logging of string pointer", "warning")))
inline OSTREAM_STRING_POINTER_USAGE_WARNING
std::ostream& operator<<(std::ostream& stream, const std::string* string_pointer) {
  return stream << static_cast<const void*>(string_pointer);
}
#pragma clang diagnostic pop

}  // namespace std
