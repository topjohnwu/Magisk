// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Support for registering benchmarks for functions.

/* Example usage:
// Define a function that executes the code to be measured a
// specified number of times:
static void BM_StringCreation(benchmark::State& state) {
  for (auto _ : state)
    std::string empty_string;
}

// Register the function as a benchmark
BENCHMARK(BM_StringCreation);

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state)
    std::string copy(x);
}
BENCHMARK(BM_StringCopy);

// Augment the main() program to invoke benchmarks if specified
// via the --benchmarks command line flag.  E.g.,
//       my_unittest --benchmark_filter=all
//       my_unittest --benchmark_filter=BM_StringCreation
//       my_unittest --benchmark_filter=String
//       my_unittest --benchmark_filter='Copy|Creation'
int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}

// Sometimes a family of microbenchmarks can be implemented with
// just one routine that takes an extra argument to specify which
// one of the family of benchmarks to run.  For example, the following
// code defines a family of microbenchmarks for measuring the speed
// of memcpy() calls of different lengths:

static void BM_memcpy(benchmark::State& state) {
  char* src = new char[state.range(0)]; char* dst = new char[state.range(0)];
  memset(src, 'x', state.range(0));
  for (auto _ : state)
    memcpy(dst, src, state.range(0));
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(state.range(0)));
  delete[] src; delete[] dst;
}
BENCHMARK(BM_memcpy)->Arg(8)->Arg(64)->Arg(512)->Arg(1<<10)->Arg(8<<10);

// The preceding code is quite repetitive, and can be replaced with the
// following short-hand.  The following invocation will pick a few
// appropriate arguments in the specified range and will generate a
// microbenchmark for each such argument.
BENCHMARK(BM_memcpy)->Range(8, 8<<10);

// You might have a microbenchmark that depends on two inputs.  For
// example, the following code defines a family of microbenchmarks for
// measuring the speed of set insertion.
static void BM_SetInsert(benchmark::State& state) {
  set<int> data;
  for (auto _ : state) {
    state.PauseTiming();
    data = ConstructRandomSet(state.range(0));
    state.ResumeTiming();
    for (int j = 0; j < state.range(1); ++j)
      data.insert(RandomNumber());
  }
}
BENCHMARK(BM_SetInsert)
   ->Args({1<<10, 128})
   ->Args({2<<10, 128})
   ->Args({4<<10, 128})
   ->Args({8<<10, 128})
   ->Args({1<<10, 512})
   ->Args({2<<10, 512})
   ->Args({4<<10, 512})
   ->Args({8<<10, 512});

// The preceding code is quite repetitive, and can be replaced with
// the following short-hand.  The following macro will pick a few
// appropriate arguments in the product of the two specified ranges
// and will generate a microbenchmark for each such pair.
BENCHMARK(BM_SetInsert)->Ranges({{1<<10, 8<<10}, {128, 512}});

// For more complex patterns of inputs, passing a custom function
// to Apply allows programmatic specification of an
// arbitrary set of arguments to run the microbenchmark on.
// The following example enumerates a dense range on
// one parameter, and a sparse range on the second.
static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int i = 0; i <= 10; ++i)
    for (int j = 32; j <= 1024*1024; j *= 8)
      b->Args({i, j});
}
BENCHMARK(BM_SetInsert)->Apply(CustomArguments);

// Templated microbenchmarks work the same way:
// Produce then consume 'size' messages 'iters' times
// Measures throughput in the absence of multiprogramming.
template <class Q> int BM_Sequential(benchmark::State& state) {
  Q q;
  typename Q::value_type v;
  for (auto _ : state) {
    for (int i = state.range(0); i--; )
      q.push(v);
    for (int e = state.range(0); e--; )
      q.Wait(&v);
  }
  // actually messages, not bytes:
  state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations())*state.range(0));
}
BENCHMARK_TEMPLATE(BM_Sequential, WaitQueue<int>)->Range(1<<0, 1<<10);

Use `Benchmark::MinTime(double t)` to set the minimum time used to run the
benchmark. This option overrides the `benchmark_min_time` flag.

void BM_test(benchmark::State& state) {
 ... body ...
}
BENCHMARK(BM_test)->MinTime(2.0); // Run for at least 2 seconds.

In a multithreaded test, it is guaranteed that none of the threads will start
until all have reached the loop start, and all will have finished before any
thread exits the loop body. As such, any global setup or teardown you want to
do can be wrapped in a check against the thread index:

static void BM_MultiThreaded(benchmark::State& state) {
  if (state.thread_index == 0) {
    // Setup code here.
  }
  for (auto _ : state) {
    // Run the test as normal.
  }
  if (state.thread_index == 0) {
    // Teardown code here.
  }
}
BENCHMARK(BM_MultiThreaded)->Threads(4);


If a benchmark runs a few milliseconds it may be hard to visually compare the
measured times, since the output data is given in nanoseconds per default. In
order to manually set the time unit, you can specify it manually:

BENCHMARK(BM_test)->Unit(benchmark::kMillisecond);
*/

#ifndef BENCHMARK_BENCHMARK_H_
#define BENCHMARK_BENCHMARK_H_

// The _MSVC_LANG check should detect Visual Studio 2015 Update 3 and newer.
#if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
#define BENCHMARK_HAS_CXX11
#endif

#include <stdint.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#if defined(BENCHMARK_HAS_CXX11)
#include <initializer_list>
#include <type_traits>
#include <utility>
#endif

#if defined(_MSC_VER)
#include <intrin.h>  // for _ReadWriteBarrier
#endif

#ifndef BENCHMARK_HAS_CXX11
#define BENCHMARK_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);                         \
  TypeName& operator=(const TypeName&)
#else
#define BENCHMARK_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;                \
  TypeName& operator=(const TypeName&) = delete
#endif

#if defined(__GNUC__)
#define BENCHMARK_UNUSED __attribute__((unused))
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
#define BENCHMARK_NOEXCEPT noexcept
#define BENCHMARK_NOEXCEPT_OP(x) noexcept(x)
#elif defined(_MSC_VER) && !defined(__clang__)
#define BENCHMARK_UNUSED
#define BENCHMARK_ALWAYS_INLINE __forceinline
#if _MSC_VER >= 1900
#define BENCHMARK_NOEXCEPT noexcept
#define BENCHMARK_NOEXCEPT_OP(x) noexcept(x)
#else
#define BENCHMARK_NOEXCEPT
#define BENCHMARK_NOEXCEPT_OP(x)
#endif
#define __func__ __FUNCTION__
#else
#define BENCHMARK_UNUSED
#define BENCHMARK_ALWAYS_INLINE
#define BENCHMARK_NOEXCEPT
#define BENCHMARK_NOEXCEPT_OP(x)
#endif

#define BENCHMARK_INTERNAL_TOSTRING2(x) #x
#define BENCHMARK_INTERNAL_TOSTRING(x) BENCHMARK_INTERNAL_TOSTRING2(x)

#if defined(__GNUC__) || defined(__clang__)
#define BENCHMARK_BUILTIN_EXPECT(x, y) __builtin_expect(x, y)
#define BENCHMARK_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
#define BENCHMARK_BUILTIN_EXPECT(x, y) x
#define BENCHMARK_DEPRECATED_MSG(msg)
#define BENCHMARK_WARNING_MSG(msg)                           \
  __pragma(message(__FILE__ "(" BENCHMARK_INTERNAL_TOSTRING( \
      __LINE__) ") : warning note: " msg))
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define BENCHMARK_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if defined(__GNUC__) || __has_builtin(__builtin_unreachable)
  #define BENCHMARK_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
  #define BENCHMARK_UNREACHABLE() __assume(false)
#else
  #define BENCHMARK_UNREACHABLE() ((void)0)
#endif

namespace benchmark {
class BenchmarkReporter;
class MemoryManager;

void Initialize(int* argc, char** argv);

// Report to stdout all arguments in 'argv' as unrecognized except the first.
// Returns true there is at least on unrecognized argument (i.e. 'argc' > 1).
bool ReportUnrecognizedArguments(int argc, char** argv);

// Generate a list of benchmarks matching the specified --benchmark_filter flag
// and if --benchmark_list_tests is specified return after printing the name
// of each matching benchmark. Otherwise run each matching benchmark and
// report the results.
//
// The second and third overload use the specified 'display_reporter' and
//  'file_reporter' respectively. 'file_reporter' will write to the file
//  specified
//   by '--benchmark_output'. If '--benchmark_output' is not given the
//  'file_reporter' is ignored.
//
// RETURNS: The number of matching benchmarks.
size_t RunSpecifiedBenchmarks();
size_t RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter);
size_t RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter,
                              BenchmarkReporter* file_reporter);

// Register a MemoryManager instance that will be used to collect and report
// allocation measurements for benchmark runs.
void RegisterMemoryManager(MemoryManager* memory_manager);

namespace internal {
class Benchmark;
class BenchmarkImp;
class BenchmarkFamilies;

void UseCharPointer(char const volatile*);

// Take ownership of the pointer and register the benchmark. Return the
// registered benchmark.
Benchmark* RegisterBenchmarkInternal(Benchmark*);

// Ensure that the standard streams are properly initialized in every TU.
int InitializeStreams();
BENCHMARK_UNUSED static int stream_init_anchor = InitializeStreams();

}  // namespace internal

#if (!defined(__GNUC__) && !defined(__clang__)) || defined(__pnacl__) || \
    defined(__EMSCRIPTEN__)
#define BENCHMARK_HAS_NO_INLINE_ASSEMBLY
#endif

// The DoNotOptimize(...) function can be used to prevent a value or
// expression from being optimized away by the compiler. This function is
// intended to add little to no overhead.
// See: https://youtu.be/nXaxk27zwlk?t=2441
#ifndef BENCHMARK_HAS_NO_INLINE_ASSEMBLY
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp& value) {
#if defined(__clang__)
  asm volatile("" : "+r,m"(value) : : "memory");
#else
  asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

// Force the compiler to flush pending writes to global memory. Acts as an
// effective read/write barrier
inline BENCHMARK_ALWAYS_INLINE void ClobberMemory() {
  asm volatile("" : : : "memory");
}
#elif defined(_MSC_VER)
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  internal::UseCharPointer(&reinterpret_cast<char const volatile&>(value));
  _ReadWriteBarrier();
}

inline BENCHMARK_ALWAYS_INLINE void ClobberMemory() { _ReadWriteBarrier(); }
#else
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  internal::UseCharPointer(&reinterpret_cast<char const volatile&>(value));
}
// FIXME Add ClobberMemory() for non-gnu and non-msvc compilers
#endif

// This class is used for user-defined counters.
class Counter {
 public:
  enum Flags {
    kDefaults = 0,
    // Mark the counter as a rate. It will be presented divided
    // by the duration of the benchmark.
    kIsRate = 1U << 0U,
    // Mark the counter as a thread-average quantity. It will be
    // presented divided by the number of threads.
    kAvgThreads = 1U << 1U,
    // Mark the counter as a thread-average rate. See above.
    kAvgThreadsRate = kIsRate | kAvgThreads,
    // Mark the counter as a constant value, valid/same for *every* iteration.
    // When reporting, it will be *multiplied* by the iteration count.
    kIsIterationInvariant = 1U << 2U,
    // Mark the counter as a constant rate.
    // When reporting, it will be *multiplied* by the iteration count
    // and then divided by the duration of the benchmark.
    kIsIterationInvariantRate = kIsRate | kIsIterationInvariant,
    // Mark the counter as a iteration-average quantity.
    // It will be presented divided by the number of iterations.
    kAvgIterations = 1U << 3U,
    // Mark the counter as a iteration-average rate. See above.
    kAvgIterationsRate = kIsRate | kAvgIterations
  };

  enum OneK {
    // 1'000 items per 1k
    kIs1000 = 1000,
    // 1'024 items per 1k
    kIs1024 = 1024
  };

  double value;
  Flags flags;
  OneK oneK;

  BENCHMARK_ALWAYS_INLINE
  Counter(double v = 0., Flags f = kDefaults, OneK k = kIs1000)
      : value(v), flags(f), oneK(k) {}

  BENCHMARK_ALWAYS_INLINE operator double const&() const { return value; }
  BENCHMARK_ALWAYS_INLINE operator double&() { return value; }
};

// A helper for user code to create unforeseen combinations of Flags, without
// having to do this cast manually each time, or providing this operator.
Counter::Flags inline operator|(const Counter::Flags& LHS,
                                const Counter::Flags& RHS) {
  return static_cast<Counter::Flags>(static_cast<int>(LHS) |
                                     static_cast<int>(RHS));
}

// This is the container for the user-defined counters.
typedef std::map<std::string, Counter> UserCounters;

// TimeUnit is passed to a benchmark in order to specify the order of magnitude
// for the measured time.
enum TimeUnit { kNanosecond, kMicrosecond, kMillisecond };

// BigO is passed to a benchmark in order to specify the asymptotic
// computational
// complexity for the benchmark. In case oAuto is selected, complexity will be
// calculated automatically to the best fit.
enum BigO { oNone, o1, oN, oNSquared, oNCubed, oLogN, oNLogN, oAuto, oLambda };

// BigOFunc is passed to a benchmark in order to specify the asymptotic
// computational complexity for the benchmark.
typedef double(BigOFunc)(int64_t);

// StatisticsFunc is passed to a benchmark in order to compute some descriptive
// statistics over all the measurements of some type
typedef double(StatisticsFunc)(const std::vector<double>&);

struct Statistics {
  std::string name_;
  StatisticsFunc* compute_;

  Statistics(const std::string& name, StatisticsFunc* compute)
      : name_(name), compute_(compute) {}
};

namespace internal {
struct BenchmarkInstance;
class ThreadTimer;
class ThreadManager;

enum AggregationReportMode
#if defined(BENCHMARK_HAS_CXX11)
    : unsigned
#else
#endif
{
  // The mode has not been manually specified
  ARM_Unspecified = 0,
  // The mode is user-specified.
  // This may or may not be set when the following bit-flags are set.
  ARM_Default = 1U << 0U,
  // File reporter should only output aggregates.
  ARM_FileReportAggregatesOnly = 1U << 1U,
  // Display reporter should only output aggregates
  ARM_DisplayReportAggregatesOnly = 1U << 2U,
  // Both reporters should only display aggregates.
  ARM_ReportAggregatesOnly =
      ARM_FileReportAggregatesOnly | ARM_DisplayReportAggregatesOnly
};

}  // namespace internal

// State is passed to a running Benchmark and contains state for the
// benchmark to use.
class State {
 public:
  struct StateIterator;
  friend struct StateIterator;

  // Returns iterators used to run each iteration of a benchmark using a
  // C++11 ranged-based for loop. These functions should not be called directly.
  //
  // REQUIRES: The benchmark has not started running yet. Neither begin nor end
  // have been called previously.
  //
  // NOTE: KeepRunning may not be used after calling either of these functions.
  BENCHMARK_ALWAYS_INLINE StateIterator begin();
  BENCHMARK_ALWAYS_INLINE StateIterator end();

  // Returns true if the benchmark should continue through another iteration.
  // NOTE: A benchmark may not return from the test until KeepRunning() has
  // returned false.
  bool KeepRunning();

  // Returns true iff the benchmark should run n more iterations.
  // REQUIRES: 'n' > 0.
  // NOTE: A benchmark must not return from the test until KeepRunningBatch()
  // has returned false.
  // NOTE: KeepRunningBatch() may overshoot by up to 'n' iterations.
  //
  // Intended usage:
  //   while (state.KeepRunningBatch(1000)) {
  //     // process 1000 elements
  //   }
  bool KeepRunningBatch(size_t n);

  // REQUIRES: timer is running and 'SkipWithError(...)' has not been called
  //           by the current thread.
  // Stop the benchmark timer.  If not called, the timer will be
  // automatically stopped after the last iteration of the benchmark loop.
  //
  // For threaded benchmarks the PauseTiming() function only pauses the timing
  // for the current thread.
  //
  // NOTE: The "real time" measurement is per-thread. If different threads
  // report different measurements the largest one is reported.
  //
  // NOTE: PauseTiming()/ResumeTiming() are relatively
  // heavyweight, and so their use should generally be avoided
  // within each benchmark iteration, if possible.
  void PauseTiming();

  // REQUIRES: timer is not running and 'SkipWithError(...)' has not been called
  //           by the current thread.
  // Start the benchmark timer.  The timer is NOT running on entrance to the
  // benchmark function. It begins running after control flow enters the
  // benchmark loop.
  //
  // NOTE: PauseTiming()/ResumeTiming() are relatively
  // heavyweight, and so their use should generally be avoided
  // within each benchmark iteration, if possible.
  void ResumeTiming();

  // REQUIRES: 'SkipWithError(...)' has not been called previously by the
  //            current thread.
  // Report the benchmark as resulting in an error with the specified 'msg'.
  // After this call the user may explicitly 'return' from the benchmark.
  //
  // If the ranged-for style of benchmark loop is used, the user must explicitly
  // break from the loop, otherwise all future iterations will be run.
  // If the 'KeepRunning()' loop is used the current thread will automatically
  // exit the loop at the end of the current iteration.
  //
  // For threaded benchmarks only the current thread stops executing and future
  // calls to `KeepRunning()` will block until all threads have completed
  // the `KeepRunning()` loop. If multiple threads report an error only the
  // first error message is used.
  //
  // NOTE: Calling 'SkipWithError(...)' does not cause the benchmark to exit
  // the current scope immediately. If the function is called from within
  // the 'KeepRunning()' loop the current iteration will finish. It is the users
  // responsibility to exit the scope as needed.
  void SkipWithError(const char* msg);

  // REQUIRES: called exactly once per iteration of the benchmarking loop.
  // Set the manually measured time for this benchmark iteration, which
  // is used instead of automatically measured time if UseManualTime() was
  // specified.
  //
  // For threaded benchmarks the final value will be set to the largest
  // reported values.
  void SetIterationTime(double seconds);

  // Set the number of bytes processed by the current benchmark
  // execution.  This routine is typically called once at the end of a
  // throughput oriented benchmark.
  //
  // REQUIRES: a benchmark has exited its benchmarking loop.
  BENCHMARK_ALWAYS_INLINE
  void SetBytesProcessed(int64_t bytes) {
    counters["bytes_per_second"] =
        Counter(static_cast<double>(bytes), Counter::kIsRate, Counter::kIs1024);
  }

  BENCHMARK_ALWAYS_INLINE
  int64_t bytes_processed() const {
    if (counters.find("bytes_per_second") != counters.end())
      return static_cast<int64_t>(counters.at("bytes_per_second"));
    return 0;
  }

  // If this routine is called with complexity_n > 0 and complexity report is
  // requested for the
  // family benchmark, then current benchmark will be part of the computation
  // and complexity_n will
  // represent the length of N.
  BENCHMARK_ALWAYS_INLINE
  void SetComplexityN(int64_t complexity_n) { complexity_n_ = complexity_n; }

  BENCHMARK_ALWAYS_INLINE
  int64_t complexity_length_n() { return complexity_n_; }

  // If this routine is called with items > 0, then an items/s
  // label is printed on the benchmark report line for the currently
  // executing benchmark. It is typically called at the end of a processing
  // benchmark where a processing items/second output is desired.
  //
  // REQUIRES: a benchmark has exited its benchmarking loop.
  BENCHMARK_ALWAYS_INLINE
  void SetItemsProcessed(int64_t items) {
    counters["items_per_second"] =
        Counter(static_cast<double>(items), benchmark::Counter::kIsRate);
  }

  BENCHMARK_ALWAYS_INLINE
  int64_t items_processed() const {
    if (counters.find("items_per_second") != counters.end())
      return static_cast<int64_t>(counters.at("items_per_second"));
    return 0;
  }

  // If this routine is called, the specified label is printed at the
  // end of the benchmark report line for the currently executing
  // benchmark.  Example:
  //  static void BM_Compress(benchmark::State& state) {
  //    ...
  //    double compress = input_size / output_size;
  //    state.SetLabel(StrFormat("compress:%.1f%%", 100.0*compression));
  //  }
  // Produces output that looks like:
  //  BM_Compress   50         50   14115038  compress:27.3%
  //
  // REQUIRES: a benchmark has exited its benchmarking loop.
  void SetLabel(const char* label);

  void BENCHMARK_ALWAYS_INLINE SetLabel(const std::string& str) {
    this->SetLabel(str.c_str());
  }

  // Range arguments for this run. CHECKs if the argument has been set.
  BENCHMARK_ALWAYS_INLINE
  int64_t range(std::size_t pos = 0) const {
    assert(range_.size() > pos);
    return range_[pos];
  }

  BENCHMARK_DEPRECATED_MSG("use 'range(0)' instead")
  int64_t range_x() const { return range(0); }

  BENCHMARK_DEPRECATED_MSG("use 'range(1)' instead")
  int64_t range_y() const { return range(1); }

  BENCHMARK_ALWAYS_INLINE
  size_t iterations() const {
    if (BENCHMARK_BUILTIN_EXPECT(!started_, false)) {
      return 0;
    }
    return max_iterations - total_iterations_ + batch_leftover_;
  }

 private
     :  // items we expect on the first cache line (ie 64 bytes of the struct)
  // When total_iterations_ is 0, KeepRunning() and friends will return false.
  // May be larger than max_iterations.
  size_t total_iterations_;

  // When using KeepRunningBatch(), batch_leftover_ holds the number of
  // iterations beyond max_iters that were run. Used to track
  // completed_iterations_ accurately.
  size_t batch_leftover_;

 public:
  const size_t max_iterations;

 private:
  bool started_;
  bool finished_;
  bool error_occurred_;

 private:  // items we don't need on the first cache line
  std::vector<int64_t> range_;

  int64_t complexity_n_;

 public:
  // Container for user-defined counters.
  UserCounters counters;
  // Index of the executing thread. Values from [0, threads).
  const int thread_index;
  // Number of threads concurrently executing the benchmark.
  const int threads;

 private:
  State(size_t max_iters, const std::vector<int64_t>& ranges, int thread_i,
        int n_threads, internal::ThreadTimer* timer,
        internal::ThreadManager* manager);

  void StartKeepRunning();
  // Implementation of KeepRunning() and KeepRunningBatch().
  // is_batch must be true unless n is 1.
  bool KeepRunningInternal(size_t n, bool is_batch);
  void FinishKeepRunning();
  internal::ThreadTimer* timer_;
  internal::ThreadManager* manager_;

  friend struct internal::BenchmarkInstance;
};

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunning() {
  return KeepRunningInternal(1, /*is_batch=*/false);
}

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunningBatch(size_t n) {
  return KeepRunningInternal(n, /*is_batch=*/true);
}

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunningInternal(size_t n,
                                                               bool is_batch) {
  // total_iterations_ is set to 0 by the constructor, and always set to a
  // nonzero value by StartKepRunning().
  assert(n > 0);
  // n must be 1 unless is_batch is true.
  assert(is_batch || n == 1);
  if (BENCHMARK_BUILTIN_EXPECT(total_iterations_ >= n, true)) {
    total_iterations_ -= n;
    return true;
  }
  if (!started_) {
    StartKeepRunning();
    if (!error_occurred_ && total_iterations_ >= n) {
      total_iterations_ -= n;
      return true;
    }
  }
  // For non-batch runs, total_iterations_ must be 0 by now.
  if (is_batch && total_iterations_ != 0) {
    batch_leftover_ = n - total_iterations_;
    total_iterations_ = 0;
    return true;
  }
  FinishKeepRunning();
  return false;
}

struct State::StateIterator {
  struct BENCHMARK_UNUSED Value {};
  typedef std::forward_iterator_tag iterator_category;
  typedef Value value_type;
  typedef Value reference;
  typedef Value pointer;
  typedef std::ptrdiff_t difference_type;

 private:
  friend class State;
  BENCHMARK_ALWAYS_INLINE
  StateIterator() : cached_(0), parent_() {}

  BENCHMARK_ALWAYS_INLINE
  explicit StateIterator(State* st)
      : cached_(st->error_occurred_ ? 0 : st->max_iterations), parent_(st) {}

 public:
  BENCHMARK_ALWAYS_INLINE
  Value operator*() const { return Value(); }

  BENCHMARK_ALWAYS_INLINE
  StateIterator& operator++() {
    assert(cached_ > 0);
    --cached_;
    return *this;
  }

  BENCHMARK_ALWAYS_INLINE
  bool operator!=(StateIterator const&) const {
    if (BENCHMARK_BUILTIN_EXPECT(cached_ != 0, true)) return true;
    parent_->FinishKeepRunning();
    return false;
  }

 private:
  size_t cached_;
  State* const parent_;
};

inline BENCHMARK_ALWAYS_INLINE State::StateIterator State::begin() {
  return StateIterator(this);
}
inline BENCHMARK_ALWAYS_INLINE State::StateIterator State::end() {
  StartKeepRunning();
  return StateIterator();
}

namespace internal {

typedef void(Function)(State&);

// ------------------------------------------------------
// Benchmark registration object.  The BENCHMARK() macro expands
// into an internal::Benchmark* object.  Various methods can
// be called on this object to change the properties of the benchmark.
// Each method returns "this" so that multiple method calls can
// chained into one expression.
class Benchmark {
 public:
  virtual ~Benchmark();

  // Note: the following methods all return "this" so that multiple
  // method calls can be chained together in one expression.

  // Run this benchmark once with "x" as the extra argument passed
  // to the function.
  // REQUIRES: The function passed to the constructor must accept an arg1.
  Benchmark* Arg(int64_t x);

  // Run this benchmark with the given time unit for the generated output report
  Benchmark* Unit(TimeUnit unit);

  // Run this benchmark once for a number of values picked from the
  // range [start..limit].  (start and limit are always picked.)
  // REQUIRES: The function passed to the constructor must accept an arg1.
  Benchmark* Range(int64_t start, int64_t limit);

  // Run this benchmark once for all values in the range [start..limit] with
  // specific step
  // REQUIRES: The function passed to the constructor must accept an arg1.
  Benchmark* DenseRange(int64_t start, int64_t limit, int step = 1);

  // Run this benchmark once with "args" as the extra arguments passed
  // to the function.
  // REQUIRES: The function passed to the constructor must accept arg1, arg2 ...
  Benchmark* Args(const std::vector<int64_t>& args);

  // Equivalent to Args({x, y})
  // NOTE: This is a legacy C++03 interface provided for compatibility only.
  //   New code should use 'Args'.
  Benchmark* ArgPair(int64_t x, int64_t y) {
    std::vector<int64_t> args;
    args.push_back(x);
    args.push_back(y);
    return Args(args);
  }

  // Run this benchmark once for a number of values picked from the
  // ranges [start..limit].  (starts and limits are always picked.)
  // REQUIRES: The function passed to the constructor must accept arg1, arg2 ...
  Benchmark* Ranges(const std::vector<std::pair<int64_t, int64_t> >& ranges);

  // Equivalent to ArgNames({name})
  Benchmark* ArgName(const std::string& name);

  // Set the argument names to display in the benchmark name. If not called,
  // only argument values will be shown.
  Benchmark* ArgNames(const std::vector<std::string>& names);

  // Equivalent to Ranges({{lo1, hi1}, {lo2, hi2}}).
  // NOTE: This is a legacy C++03 interface provided for compatibility only.
  //   New code should use 'Ranges'.
  Benchmark* RangePair(int64_t lo1, int64_t hi1, int64_t lo2, int64_t hi2) {
    std::vector<std::pair<int64_t, int64_t> > ranges;
    ranges.push_back(std::make_pair(lo1, hi1));
    ranges.push_back(std::make_pair(lo2, hi2));
    return Ranges(ranges);
  }

  // Pass this benchmark object to *func, which can customize
  // the benchmark by calling various methods like Arg, Args,
  // Threads, etc.
  Benchmark* Apply(void (*func)(Benchmark* benchmark));

  // Set the range multiplier for non-dense range. If not called, the range
  // multiplier kRangeMultiplier will be used.
  Benchmark* RangeMultiplier(int multiplier);

  // Set the minimum amount of time to use when running this benchmark. This
  // option overrides the `benchmark_min_time` flag.
  // REQUIRES: `t > 0` and `Iterations` has not been called on this benchmark.
  Benchmark* MinTime(double t);

  // Specify the amount of iterations that should be run by this benchmark.
  // REQUIRES: 'n > 0' and `MinTime` has not been called on this benchmark.
  //
  // NOTE: This function should only be used when *exact* iteration control is
  //   needed and never to control or limit how long a benchmark runs, where
  // `--benchmark_min_time=N` or `MinTime(...)` should be used instead.
  Benchmark* Iterations(size_t n);

  // Specify the amount of times to repeat this benchmark. This option overrides
  // the `benchmark_repetitions` flag.
  // REQUIRES: `n > 0`
  Benchmark* Repetitions(int n);

  // Specify if each repetition of the benchmark should be reported separately
  // or if only the final statistics should be reported. If the benchmark
  // is not repeated then the single result is always reported.
  // Applies to *ALL* reporters (display and file).
  Benchmark* ReportAggregatesOnly(bool value = true);

  // Same as ReportAggregatesOnly(), but applies to display reporter only.
  Benchmark* DisplayAggregatesOnly(bool value = true);

  // If a particular benchmark is I/O bound, runs multiple threads internally or
  // if for some reason CPU timings are not representative, call this method. If
  // called, the elapsed time will be used to control how many iterations are
  // run, and in the printing of items/second or MB/seconds values.  If not
  // called, the cpu time used by the benchmark will be used.
  Benchmark* UseRealTime();

  // If a benchmark must measure time manually (e.g. if GPU execution time is
  // being
  // measured), call this method. If called, each benchmark iteration should
  // call
  // SetIterationTime(seconds) to report the measured time, which will be used
  // to control how many iterations are run, and in the printing of items/second
  // or MB/second values.
  Benchmark* UseManualTime();

  // Set the asymptotic computational complexity for the benchmark. If called
  // the asymptotic computational complexity will be shown on the output.
  Benchmark* Complexity(BigO complexity = benchmark::oAuto);

  // Set the asymptotic computational complexity for the benchmark. If called
  // the asymptotic computational complexity will be shown on the output.
  Benchmark* Complexity(BigOFunc* complexity);

  // Add this statistics to be computed over all the values of benchmark run
  Benchmark* ComputeStatistics(std::string name, StatisticsFunc* statistics);

  // Support for running multiple copies of the same benchmark concurrently
  // in multiple threads.  This may be useful when measuring the scaling
  // of some piece of code.

  // Run one instance of this benchmark concurrently in t threads.
  Benchmark* Threads(int t);

  // Pick a set of values T from [min_threads,max_threads].
  // min_threads and max_threads are always included in T.  Run this
  // benchmark once for each value in T.  The benchmark run for a
  // particular value t consists of t threads running the benchmark
  // function concurrently.  For example, consider:
  //    BENCHMARK(Foo)->ThreadRange(1,16);
  // This will run the following benchmarks:
  //    Foo in 1 thread
  //    Foo in 2 threads
  //    Foo in 4 threads
  //    Foo in 8 threads
  //    Foo in 16 threads
  Benchmark* ThreadRange(int min_threads, int max_threads);

  // For each value n in the range, run this benchmark once using n threads.
  // min_threads and max_threads are always included in the range.
  // stride specifies the increment. E.g. DenseThreadRange(1, 8, 3) starts
  // a benchmark with 1, 4, 7 and 8 threads.
  Benchmark* DenseThreadRange(int min_threads, int max_threads, int stride = 1);

  // Equivalent to ThreadRange(NumCPUs(), NumCPUs())
  Benchmark* ThreadPerCpu();

  virtual void Run(State& state) = 0;

 protected:
  explicit Benchmark(const char* name);
  Benchmark(Benchmark const&);
  void SetName(const char* name);

  int ArgsCnt() const;

 private:
  friend class BenchmarkFamilies;

  std::string name_;
  AggregationReportMode aggregation_report_mode_;
  std::vector<std::string> arg_names_;       // Args for all benchmark runs
  std::vector<std::vector<int64_t> > args_;  // Args for all benchmark runs
  TimeUnit time_unit_;
  int range_multiplier_;
  double min_time_;
  size_t iterations_;
  int repetitions_;
  bool use_real_time_;
  bool use_manual_time_;
  BigO complexity_;
  BigOFunc* complexity_lambda_;
  std::vector<Statistics> statistics_;
  std::vector<int> thread_counts_;

  Benchmark& operator=(Benchmark const&);
};

}  // namespace internal

// Create and register a benchmark with the specified 'name' that invokes
// the specified functor 'fn'.
//
// RETURNS: A pointer to the registered benchmark.
internal::Benchmark* RegisterBenchmark(const char* name,
                                       internal::Function* fn);

#if defined(BENCHMARK_HAS_CXX11)
template <class Lambda>
internal::Benchmark* RegisterBenchmark(const char* name, Lambda&& fn);
#endif

// Remove all registered benchmarks. All pointers to previously registered
// benchmarks are invalidated.
void ClearRegisteredBenchmarks();

namespace internal {
// The class used to hold all Benchmarks created from static function.
// (ie those created using the BENCHMARK(...) macros.
class FunctionBenchmark : public Benchmark {
 public:
  FunctionBenchmark(const char* name, Function* func)
      : Benchmark(name), func_(func) {}

  virtual void Run(State& st);

 private:
  Function* func_;
};

#ifdef BENCHMARK_HAS_CXX11
template <class Lambda>
class LambdaBenchmark : public Benchmark {
 public:
  virtual void Run(State& st) { lambda_(st); }

 private:
  template <class OLambda>
  LambdaBenchmark(const char* name, OLambda&& lam)
      : Benchmark(name), lambda_(std::forward<OLambda>(lam)) {}

  LambdaBenchmark(LambdaBenchmark const&) = delete;

 private:
  template <class Lam>
  friend Benchmark* ::benchmark::RegisterBenchmark(const char*, Lam&&);

  Lambda lambda_;
};
#endif

}  // namespace internal

inline internal::Benchmark* RegisterBenchmark(const char* name,
                                              internal::Function* fn) {
  return internal::RegisterBenchmarkInternal(
      ::new internal::FunctionBenchmark(name, fn));
}

#ifdef BENCHMARK_HAS_CXX11
template <class Lambda>
internal::Benchmark* RegisterBenchmark(const char* name, Lambda&& fn) {
  using BenchType =
      internal::LambdaBenchmark<typename std::decay<Lambda>::type>;
  return internal::RegisterBenchmarkInternal(
      ::new BenchType(name, std::forward<Lambda>(fn)));
}
#endif

#if defined(BENCHMARK_HAS_CXX11) && \
    (!defined(BENCHMARK_GCC_VERSION) || BENCHMARK_GCC_VERSION >= 409)
template <class Lambda, class... Args>
internal::Benchmark* RegisterBenchmark(const char* name, Lambda&& fn,
                                       Args&&... args) {
  return benchmark::RegisterBenchmark(
      name, [=](benchmark::State& st) { fn(st, args...); });
}
#else
#define BENCHMARK_HAS_NO_VARIADIC_REGISTER_BENCHMARK
#endif

// The base class for all fixture tests.
class Fixture : public internal::Benchmark {
 public:
  Fixture() : internal::Benchmark("") {}

  virtual void Run(State& st) {
    this->SetUp(st);
    this->BenchmarkCase(st);
    this->TearDown(st);
  }

  // These will be deprecated ...
  virtual void SetUp(const State&) {}
  virtual void TearDown(const State&) {}
  // ... In favor of these.
  virtual void SetUp(State& st) { SetUp(const_cast<const State&>(st)); }
  virtual void TearDown(State& st) { TearDown(const_cast<const State&>(st)); }

 protected:
  virtual void BenchmarkCase(State&) = 0;
};

}  // namespace benchmark

// ------------------------------------------------------
// Macro to register benchmarks

// Check that __COUNTER__ is defined and that __COUNTER__ increases by 1
// every time it is expanded. X + 1 == X + 0 is used in case X is defined to be
// empty. If X is empty the expression becomes (+1 == +0).
#if defined(__COUNTER__) && (__COUNTER__ + 1 == __COUNTER__ + 0)
#define BENCHMARK_PRIVATE_UNIQUE_ID __COUNTER__
#else
#define BENCHMARK_PRIVATE_UNIQUE_ID __LINE__
#endif

// Helpers for generating unique variable names
#define BENCHMARK_PRIVATE_NAME(n) \
  BENCHMARK_PRIVATE_CONCAT(_benchmark_, BENCHMARK_PRIVATE_UNIQUE_ID, n)
#define BENCHMARK_PRIVATE_CONCAT(a, b, c) BENCHMARK_PRIVATE_CONCAT2(a, b, c)
#define BENCHMARK_PRIVATE_CONCAT2(a, b, c) a##b##c

#define BENCHMARK_PRIVATE_DECLARE(n)                                 \
  static ::benchmark::internal::Benchmark* BENCHMARK_PRIVATE_NAME(n) \
      BENCHMARK_UNUSED

#define BENCHMARK(n)                                     \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          new ::benchmark::internal::FunctionBenchmark(#n, n)))

// Old-style macros
#define BENCHMARK_WITH_ARG(n, a) BENCHMARK(n)->Arg((a))
#define BENCHMARK_WITH_ARG2(n, a1, a2) BENCHMARK(n)->Args({(a1), (a2)})
#define BENCHMARK_WITH_UNIT(n, t) BENCHMARK(n)->Unit((t))
#define BENCHMARK_RANGE(n, lo, hi) BENCHMARK(n)->Range((lo), (hi))
#define BENCHMARK_RANGE2(n, l1, h1, l2, h2) \
  BENCHMARK(n)->RangePair({{(l1), (h1)}, {(l2), (h2)}})

#ifdef BENCHMARK_HAS_CXX11

// Register a benchmark which invokes the function specified by `func`
// with the additional arguments specified by `...`.
//
// For example:
//
// template <class ...ExtraArgs>`
// void BM_takes_args(benchmark::State& state, ExtraArgs&&... extra_args) {
//  [...]
//}
// /* Registers a benchmark named "BM_takes_args/int_string_test` */
// BENCHMARK_CAPTURE(BM_takes_args, int_string_test, 42, std::string("abc"));
#define BENCHMARK_CAPTURE(func, test_case_name, ...)     \
  BENCHMARK_PRIVATE_DECLARE(func) =                      \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          new ::benchmark::internal::FunctionBenchmark(  \
              #func "/" #test_case_name,                 \
              [](::benchmark::State& st) { func(st, __VA_ARGS__); })))

#endif  // BENCHMARK_HAS_CXX11

// This will register a benchmark for a templatized function.  For example:
//
// template<int arg>
// void BM_Foo(int iters);
//
// BENCHMARK_TEMPLATE(BM_Foo, 1);
//
// will register BM_Foo<1> as a benchmark.
#define BENCHMARK_TEMPLATE1(n, a)                        \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          new ::benchmark::internal::FunctionBenchmark(#n "<" #a ">", n<a>)))

#define BENCHMARK_TEMPLATE2(n, a, b)                                         \
  BENCHMARK_PRIVATE_DECLARE(n) =                                             \
      (::benchmark::internal::RegisterBenchmarkInternal(                     \
          new ::benchmark::internal::FunctionBenchmark(#n "<" #a "," #b ">", \
                                                       n<a, b>)))

#ifdef BENCHMARK_HAS_CXX11
#define BENCHMARK_TEMPLATE(n, ...)                       \
  BENCHMARK_PRIVATE_DECLARE(n) =                         \
      (::benchmark::internal::RegisterBenchmarkInternal( \
          new ::benchmark::internal::FunctionBenchmark(  \
              #n "<" #__VA_ARGS__ ">", n<__VA_ARGS__>)))
#else
#define BENCHMARK_TEMPLATE(n, a) BENCHMARK_TEMPLATE1(n, a)
#endif

#define BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method)        \
  class BaseClass##_##Method##_Benchmark : public BaseClass { \
   public:                                                    \
    BaseClass##_##Method##_Benchmark() : BaseClass() {        \
      this->SetName(#BaseClass "/" #Method);                  \
    }                                                         \
                                                              \
   protected:                                                 \
    virtual void BenchmarkCase(::benchmark::State&);          \
  };

#define BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  class BaseClass##_##Method##_Benchmark : public BaseClass<a> {    \
   public:                                                          \
    BaseClass##_##Method##_Benchmark() : BaseClass<a>() {           \
      this->SetName(#BaseClass "<" #a ">/" #Method);                \
    }                                                               \
                                                                    \
   protected:                                                       \
    virtual void BenchmarkCase(::benchmark::State&);                \
  };

#define BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  class BaseClass##_##Method##_Benchmark : public BaseClass<a, b> {    \
   public:                                                             \
    BaseClass##_##Method##_Benchmark() : BaseClass<a, b>() {           \
      this->SetName(#BaseClass "<" #a "," #b ">/" #Method);            \
    }                                                                  \
                                                                       \
   protected:                                                          \
    virtual void BenchmarkCase(::benchmark::State&);                   \
  };

#ifdef BENCHMARK_HAS_CXX11
#define BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, ...)       \
  class BaseClass##_##Method##_Benchmark : public BaseClass<__VA_ARGS__> { \
   public:                                                                 \
    BaseClass##_##Method##_Benchmark() : BaseClass<__VA_ARGS__>() {        \
      this->SetName(#BaseClass "<" #__VA_ARGS__ ">/" #Method);             \
    }                                                                      \
                                                                           \
   protected:                                                              \
    virtual void BenchmarkCase(::benchmark::State&);                       \
  };
#else
#define BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(n, a) \
  BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(n, a)
#endif

#define BENCHMARK_DEFINE_F(BaseClass, Method)    \
  BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method) \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#define BENCHMARK_TEMPLATE1_DEFINE_F(BaseClass, Method, a)    \
  BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#define BENCHMARK_TEMPLATE2_DEFINE_F(BaseClass, Method, a, b)    \
  BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#ifdef BENCHMARK_HAS_CXX11
#define BENCHMARK_TEMPLATE_DEFINE_F(BaseClass, Method, ...)            \
  BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, __VA_ARGS__) \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase
#else
#define BENCHMARK_TEMPLATE_DEFINE_F(BaseClass, Method, a) \
  BENCHMARK_TEMPLATE1_DEFINE_F(BaseClass, Method, a)
#endif

#define BENCHMARK_REGISTER_F(BaseClass, Method) \
  BENCHMARK_PRIVATE_REGISTER_F(BaseClass##_##Method##_Benchmark)

#define BENCHMARK_PRIVATE_REGISTER_F(TestName) \
  BENCHMARK_PRIVATE_DECLARE(TestName) =        \
      (::benchmark::internal::RegisterBenchmarkInternal(new TestName()))

// This macro will define and register a benchmark within a fixture class.
#define BENCHMARK_F(BaseClass, Method)           \
  BENCHMARK_PRIVATE_DECLARE_F(BaseClass, Method) \
  BENCHMARK_REGISTER_F(BaseClass, Method);       \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#define BENCHMARK_TEMPLATE1_F(BaseClass, Method, a)           \
  BENCHMARK_TEMPLATE1_PRIVATE_DECLARE_F(BaseClass, Method, a) \
  BENCHMARK_REGISTER_F(BaseClass, Method);                    \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#define BENCHMARK_TEMPLATE2_F(BaseClass, Method, a, b)           \
  BENCHMARK_TEMPLATE2_PRIVATE_DECLARE_F(BaseClass, Method, a, b) \
  BENCHMARK_REGISTER_F(BaseClass, Method);                       \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase

#ifdef BENCHMARK_HAS_CXX11
#define BENCHMARK_TEMPLATE_F(BaseClass, Method, ...)                   \
  BENCHMARK_TEMPLATE_PRIVATE_DECLARE_F(BaseClass, Method, __VA_ARGS__) \
  BENCHMARK_REGISTER_F(BaseClass, Method);                             \
  void BaseClass##_##Method##_Benchmark::BenchmarkCase
#else
#define BENCHMARK_TEMPLATE_F(BaseClass, Method, a) \
  BENCHMARK_TEMPLATE1_F(BaseClass, Method, a)
#endif

// Helper macro to create a main routine in a test that runs the benchmarks
#define BENCHMARK_MAIN()                                                \
  int main(int argc, char** argv) {                                     \
    ::benchmark::Initialize(&argc, argv);                               \
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1; \
    ::benchmark::RunSpecifiedBenchmarks();                              \
  }                                                                     \
  int main(int, char**)

// ------------------------------------------------------
// Benchmark Reporters

namespace benchmark {

struct CPUInfo {
  struct CacheInfo {
    std::string type;
    int level;
    int size;
    int num_sharing;
  };

  int num_cpus;
  double cycles_per_second;
  std::vector<CacheInfo> caches;
  bool scaling_enabled;
  std::vector<double> load_avg;

  static const CPUInfo& Get();

 private:
  CPUInfo();
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(CPUInfo);
};

//Adding Struct for System Information
struct SystemInfo {
  std::string name;
  static const SystemInfo& Get();
 private:
  SystemInfo();
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(SystemInfo);
};

// Interface for custom benchmark result printers.
// By default, benchmark reports are printed to stdout. However an application
// can control the destination of the reports by calling
// RunSpecifiedBenchmarks and passing it a custom reporter object.
// The reporter object must implement the following interface.
class BenchmarkReporter {
 public:
  struct Context {
    CPUInfo const& cpu_info;
    SystemInfo const& sys_info;
    // The number of chars in the longest benchmark name.
    size_t name_field_width;
    static const char* executable_name;
    Context();
  };

  struct Run {
    enum RunType { RT_Iteration, RT_Aggregate };

    Run()
        : run_type(RT_Iteration),
          error_occurred(false),
          iterations(1),
          time_unit(kNanosecond),
          real_accumulated_time(0),
          cpu_accumulated_time(0),
          max_heapbytes_used(0),
          complexity(oNone),
          complexity_lambda(),
          complexity_n(0),
          report_big_o(false),
          report_rms(false),
          counters(),
          has_memory_result(false),
          allocs_per_iter(0.0),
          max_bytes_used(0) {}

    std::string benchmark_name() const;
    std::string run_name;
    RunType run_type;          // is this a measurement, or an aggregate?
    std::string aggregate_name;
    std::string report_label;  // Empty if not set by benchmark.
    bool error_occurred;
    std::string error_message;

    int64_t iterations;
    TimeUnit time_unit;
    double real_accumulated_time;
    double cpu_accumulated_time;

    // Return a value representing the real time per iteration in the unit
    // specified by 'time_unit'.
    // NOTE: If 'iterations' is zero the returned value represents the
    // accumulated time.
    double GetAdjustedRealTime() const;

    // Return a value representing the cpu time per iteration in the unit
    // specified by 'time_unit'.
    // NOTE: If 'iterations' is zero the returned value represents the
    // accumulated time.
    double GetAdjustedCPUTime() const;

    // This is set to 0.0 if memory tracing is not enabled.
    double max_heapbytes_used;

    // Keep track of arguments to compute asymptotic complexity
    BigO complexity;
    BigOFunc* complexity_lambda;
    int64_t complexity_n;

    // what statistics to compute from the measurements
    const std::vector<Statistics>* statistics;

    // Inform print function whether the current run is a complexity report
    bool report_big_o;
    bool report_rms;

    UserCounters counters;

    // Memory metrics.
    bool has_memory_result;
    double allocs_per_iter;
    int64_t max_bytes_used;
  };

  // Construct a BenchmarkReporter with the output stream set to 'std::cout'
  // and the error stream set to 'std::cerr'
  BenchmarkReporter();

  // Called once for every suite of benchmarks run.
  // The parameter "context" contains information that the
  // reporter may wish to use when generating its report, for example the
  // platform under which the benchmarks are running. The benchmark run is
  // never started if this function returns false, allowing the reporter
  // to skip runs based on the context information.
  virtual bool ReportContext(const Context& context) = 0;

  // Called once for each group of benchmark runs, gives information about
  // cpu-time and heap memory usage during the benchmark run. If the group
  // of runs contained more than two entries then 'report' contains additional
  // elements representing the mean and standard deviation of those runs.
  // Additionally if this group of runs was the last in a family of benchmarks
  // 'reports' contains additional entries representing the asymptotic
  // complexity and RMS of that benchmark family.
  virtual void ReportRuns(const std::vector<Run>& report) = 0;

  // Called once and only once after ever group of benchmarks is run and
  // reported.
  virtual void Finalize() {}

  // REQUIRES: The object referenced by 'out' is valid for the lifetime
  // of the reporter.
  void SetOutputStream(std::ostream* out) {
    assert(out);
    output_stream_ = out;
  }

  // REQUIRES: The object referenced by 'err' is valid for the lifetime
  // of the reporter.
  void SetErrorStream(std::ostream* err) {
    assert(err);
    error_stream_ = err;
  }

  std::ostream& GetOutputStream() const { return *output_stream_; }

  std::ostream& GetErrorStream() const { return *error_stream_; }

  virtual ~BenchmarkReporter();

  // Write a human readable string to 'out' representing the specified
  // 'context'.
  // REQUIRES: 'out' is non-null.
  static void PrintBasicContext(std::ostream* out, Context const& context);

 private:
  std::ostream* output_stream_;
  std::ostream* error_stream_;
};

// Simple reporter that outputs benchmark data to the console. This is the
// default reporter used by RunSpecifiedBenchmarks().
class ConsoleReporter : public BenchmarkReporter {
 public:
  enum OutputOptions {
    OO_None = 0,
    OO_Color = 1,
    OO_Tabular = 2,
    OO_ColorTabular = OO_Color | OO_Tabular,
    OO_Defaults = OO_ColorTabular
  };
  explicit ConsoleReporter(OutputOptions opts_ = OO_Defaults)
      : output_options_(opts_),
        name_field_width_(0),
        prev_counters_(),
        printed_header_(false) {}

  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);

 protected:
  virtual void PrintRunData(const Run& report);
  virtual void PrintHeader(const Run& report);

  OutputOptions output_options_;
  size_t name_field_width_;
  UserCounters prev_counters_;
  bool printed_header_;
};

class JSONReporter : public BenchmarkReporter {
 public:
  JSONReporter() : first_report_(true) {}
  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);
  virtual void Finalize();

 private:
  void PrintRunData(const Run& report);

  bool first_report_;
};

class BENCHMARK_DEPRECATED_MSG("The CSV Reporter will be removed in a future release")
      CSVReporter : public BenchmarkReporter {
 public:
  CSVReporter() : printed_header_(false) {}
  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);

 private:
  void PrintRunData(const Run& report);

  bool printed_header_;
  std::set<std::string> user_counter_names_;
};

// If a MemoryManager is registered, it can be used to collect and report
// allocation metrics for a run of the benchmark.
class MemoryManager {
 public:
  struct Result {
    Result() : num_allocs(0), max_bytes_used(0) {}

    // The number of allocations made in total between Start and Stop.
    int64_t num_allocs;

    // The peak memory use between Start and Stop.
    int64_t max_bytes_used;
  };

  virtual ~MemoryManager() {}

  // Implement this to start recording allocation information.
  virtual void Start() = 0;

  // Implement this to stop recording and fill out the given Result structure.
  virtual void Stop(Result* result) = 0;
};

inline const char* GetTimeUnitString(TimeUnit unit) {
  switch (unit) {
    case kMillisecond:
      return "ms";
    case kMicrosecond:
      return "us";
    case kNanosecond:
      return "ns";
  }
  BENCHMARK_UNREACHABLE();
}

inline double GetTimeUnitMultiplier(TimeUnit unit) {
  switch (unit) {
    case kMillisecond:
      return 1e3;
    case kMicrosecond:
      return 1e6;
    case kNanosecond:
      return 1e9;
  }
  BENCHMARK_UNREACHABLE();
}

}  // namespace benchmark

#endif  // BENCHMARK_BENCHMARK_H_
