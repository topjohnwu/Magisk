#ifndef BENCHMARK_LOG_H_
#define BENCHMARK_LOG_H_

#include <iostream>
#include <ostream>

#include "benchmark/benchmark.h"

namespace benchmark {
namespace internal {

typedef std::basic_ostream<char>&(EndLType)(std::basic_ostream<char>&);

class LogType {
  friend LogType& GetNullLogInstance();
  friend LogType& GetErrorLogInstance();

  // FIXME: Add locking to output.
  template <class Tp>
  friend LogType& operator<<(LogType&, Tp const&);
  friend LogType& operator<<(LogType&, EndLType*);

 private:
  LogType(std::ostream* out) : out_(out) {}
  std::ostream* out_;
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(LogType);
};

template <class Tp>
LogType& operator<<(LogType& log, Tp const& value) {
  if (log.out_) {
    *log.out_ << value;
  }
  return log;
}

inline LogType& operator<<(LogType& log, EndLType* m) {
  if (log.out_) {
    *log.out_ << m;
  }
  return log;
}

inline int& LogLevel() {
  static int log_level = 0;
  return log_level;
}

inline LogType& GetNullLogInstance() {
  static LogType log(nullptr);
  return log;
}

inline LogType& GetErrorLogInstance() {
  static LogType log(&std::clog);
  return log;
}

inline LogType& GetLogInstanceForLevel(int level) {
  if (level <= LogLevel()) {
    return GetErrorLogInstance();
  }
  return GetNullLogInstance();
}

}  // end namespace internal
}  // end namespace benchmark

// clang-format off
#define VLOG(x)                                                               \
  (::benchmark::internal::GetLogInstanceForLevel(x) << "-- LOG(" << x << "):" \
                                                                         " ")
// clang-format on
#endif
