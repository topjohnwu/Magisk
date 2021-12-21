#ifndef BENCHMARK_TIMERS_H
#define BENCHMARK_TIMERS_H

#include <chrono>
#include <string>

namespace benchmark {

// Return the CPU usage of the current process
double ProcessCPUUsage();

// Return the CPU usage of the children of the current process
double ChildrenCPUUsage();

// Return the CPU usage of the current thread
double ThreadCPUUsage();

#if defined(HAVE_STEADY_CLOCK)
template <bool HighResIsSteady = std::chrono::high_resolution_clock::is_steady>
struct ChooseSteadyClock {
  typedef std::chrono::high_resolution_clock type;
};

template <>
struct ChooseSteadyClock<false> {
  typedef std::chrono::steady_clock type;
};
#endif

struct ChooseClockType {
#if defined(HAVE_STEADY_CLOCK)
  typedef ChooseSteadyClock<>::type type;
#else
  typedef std::chrono::high_resolution_clock type;
#endif
};

inline double ChronoClockNow() {
  typedef ChooseClockType::type ClockType;
  using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;
  return FpSeconds(ClockType::now().time_since_epoch()).count();
}

std::string LocalDateTimeString();

}  // end namespace benchmark

#endif  // BENCHMARK_TIMERS_H
