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

#include "benchmark/benchmark.h"
#include "benchmark_api_internal.h"
#include "benchmark_runner.h"
#include "internal_macros.h"

#ifndef BENCHMARK_OS_WINDOWS
#ifndef BENCHMARK_OS_FUCHSIA
#include <sys/resource.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "check.h"
#include "colorprint.h"
#include "commandlineflags.h"
#include "complexity.h"
#include "counter.h"
#include "internal_macros.h"
#include "log.h"
#include "mutex.h"
#include "re.h"
#include "statistics.h"
#include "string_util.h"
#include "thread_manager.h"
#include "thread_timer.h"

DEFINE_bool(benchmark_list_tests, false,
            "Print a list of benchmarks. This option overrides all other "
            "options.");

DEFINE_string(benchmark_filter, ".",
              "A regular expression that specifies the set of benchmarks "
              "to execute.  If this flag is empty, or if this flag is the "
              "string \"all\", all benchmarks linked into the binary are "
              "run.");

DEFINE_double(benchmark_min_time, 0.5,
              "Minimum number of seconds we should run benchmark before "
              "results are considered significant.  For cpu-time based "
              "tests, this is the lower bound on the total cpu time "
              "used by all threads that make up the test.  For real-time "
              "based tests, this is the lower bound on the elapsed time "
              "of the benchmark execution, regardless of number of "
              "threads.");

DEFINE_int32(benchmark_repetitions, 1,
             "The number of runs of each benchmark. If greater than 1, the "
             "mean and standard deviation of the runs will be reported.");

DEFINE_bool(
    benchmark_report_aggregates_only, false,
    "Report the result of each benchmark repetitions. When 'true' is specified "
    "only the mean, standard deviation, and other statistics are reported for "
    "repeated benchmarks. Affects all reporters.");

DEFINE_bool(
    benchmark_display_aggregates_only, false,
    "Display the result of each benchmark repetitions. When 'true' is "
    "specified only the mean, standard deviation, and other statistics are "
    "displayed for repeated benchmarks. Unlike "
    "benchmark_report_aggregates_only, only affects the display reporter, but "
    "*NOT* file reporter, which will still contain all the output.");

DEFINE_string(benchmark_format, "console",
              "The format to use for console output. Valid values are "
              "'console', 'json', or 'csv'.");

DEFINE_string(benchmark_out_format, "json",
              "The format to use for file output. Valid values are "
              "'console', 'json', or 'csv'.");

DEFINE_string(benchmark_out, "", "The file to write additional output to");

DEFINE_string(benchmark_color, "auto",
              "Whether to use colors in the output.  Valid values: "
              "'true'/'yes'/1, 'false'/'no'/0, and 'auto'. 'auto' means to use "
              "colors if the output is being sent to a terminal and the TERM "
              "environment variable is set to a terminal type that supports "
              "colors.");

DEFINE_bool(benchmark_counters_tabular, false,
            "Whether to use tabular format when printing user counters to "
            "the console.  Valid values: 'true'/'yes'/1, 'false'/'no'/0."
            "Defaults to false.");

DEFINE_int32(v, 0, "The level of verbose logging to output");

namespace benchmark {

namespace internal {

// FIXME: wouldn't LTO mess this up?
void UseCharPointer(char const volatile*) {}

}  // namespace internal

State::State(size_t max_iters, const std::vector<int64_t>& ranges, int thread_i,
             int n_threads, internal::ThreadTimer* timer,
             internal::ThreadManager* manager)
    : total_iterations_(0),
      batch_leftover_(0),
      max_iterations(max_iters),
      started_(false),
      finished_(false),
      error_occurred_(false),
      range_(ranges),
      complexity_n_(0),
      counters(),
      thread_index(thread_i),
      threads(n_threads),
      timer_(timer),
      manager_(manager) {
  CHECK(max_iterations != 0) << "At least one iteration must be run";
  CHECK_LT(thread_index, threads) << "thread_index must be less than threads";

  // Note: The use of offsetof below is technically undefined until C++17
  // because State is not a standard layout type. However, all compilers
  // currently provide well-defined behavior as an extension (which is
  // demonstrated since constexpr evaluation must diagnose all undefined
  // behavior). However, GCC and Clang also warn about this use of offsetof,
  // which must be suppressed.
#if defined(__INTEL_COMPILER)
#pragma warning push
#pragma warning(disable:1875)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
  // Offset tests to ensure commonly accessed data is on the first cache line.
  const int cache_line_size = 64;
  static_assert(offsetof(State, error_occurred_) <=
                    (cache_line_size - sizeof(error_occurred_)),
                "");
#if defined(__INTEL_COMPILER)
#pragma warning pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

void State::PauseTiming() {
  // Add in time accumulated so far
  CHECK(started_ && !finished_ && !error_occurred_);
  timer_->StopTimer();
}

void State::ResumeTiming() {
  CHECK(started_ && !finished_ && !error_occurred_);
  timer_->StartTimer();
}

void State::SkipWithError(const char* msg) {
  CHECK(msg);
  error_occurred_ = true;
  {
    MutexLock l(manager_->GetBenchmarkMutex());
    if (manager_->results.has_error_ == false) {
      manager_->results.error_message_ = msg;
      manager_->results.has_error_ = true;
    }
  }
  total_iterations_ = 0;
  if (timer_->running()) timer_->StopTimer();
}

void State::SetIterationTime(double seconds) {
  timer_->SetIterationTime(seconds);
}

void State::SetLabel(const char* label) {
  MutexLock l(manager_->GetBenchmarkMutex());
  manager_->results.report_label_ = label;
}

void State::StartKeepRunning() {
  CHECK(!started_ && !finished_);
  started_ = true;
  total_iterations_ = error_occurred_ ? 0 : max_iterations;
  manager_->StartStopBarrier();
  if (!error_occurred_) ResumeTiming();
}

void State::FinishKeepRunning() {
  CHECK(started_ && (!finished_ || error_occurred_));
  if (!error_occurred_) {
    PauseTiming();
  }
  // Total iterations has now wrapped around past 0. Fix this.
  total_iterations_ = 0;
  finished_ = true;
  manager_->StartStopBarrier();
}

namespace internal {
namespace {

void RunBenchmarks(const std::vector<BenchmarkInstance>& benchmarks,
                   BenchmarkReporter* display_reporter,
                   BenchmarkReporter* file_reporter) {
  // Note the file_reporter can be null.
  CHECK(display_reporter != nullptr);

  // Determine the width of the name field using a minimum width of 10.
  bool might_have_aggregates = FLAGS_benchmark_repetitions > 1;
  size_t name_field_width = 10;
  size_t stat_field_width = 0;
  for (const BenchmarkInstance& benchmark : benchmarks) {
    name_field_width =
        std::max<size_t>(name_field_width, benchmark.name.size());
    might_have_aggregates |= benchmark.repetitions > 1;

    for (const auto& Stat : *benchmark.statistics)
      stat_field_width = std::max<size_t>(stat_field_width, Stat.name_.size());
  }
  if (might_have_aggregates) name_field_width += 1 + stat_field_width;

  // Print header here
  BenchmarkReporter::Context context;
  context.name_field_width = name_field_width;

  // Keep track of running times of all instances of current benchmark
  std::vector<BenchmarkReporter::Run> complexity_reports;

  // We flush streams after invoking reporter methods that write to them. This
  // ensures users get timely updates even when streams are not line-buffered.
  auto flushStreams = [](BenchmarkReporter* reporter) {
    if (!reporter) return;
    std::flush(reporter->GetOutputStream());
    std::flush(reporter->GetErrorStream());
  };

  if (display_reporter->ReportContext(context) &&
      (!file_reporter || file_reporter->ReportContext(context))) {
    flushStreams(display_reporter);
    flushStreams(file_reporter);

    for (const auto& benchmark : benchmarks) {
      RunResults run_results = RunBenchmark(benchmark, &complexity_reports);

      auto report = [&run_results](BenchmarkReporter* reporter,
                                   bool report_aggregates_only) {
        assert(reporter);
        // If there are no aggregates, do output non-aggregates.
        report_aggregates_only &= !run_results.aggregates_only.empty();
        if (!report_aggregates_only)
          reporter->ReportRuns(run_results.non_aggregates);
        if (!run_results.aggregates_only.empty())
          reporter->ReportRuns(run_results.aggregates_only);
      };

      report(display_reporter, run_results.display_report_aggregates_only);
      if (file_reporter)
        report(file_reporter, run_results.file_report_aggregates_only);

      flushStreams(display_reporter);
      flushStreams(file_reporter);
    }
  }
  display_reporter->Finalize();
  if (file_reporter) file_reporter->Finalize();
  flushStreams(display_reporter);
  flushStreams(file_reporter);
}

std::unique_ptr<BenchmarkReporter> CreateReporter(
    std::string const& name, ConsoleReporter::OutputOptions output_opts) {
  typedef std::unique_ptr<BenchmarkReporter> PtrType;
  if (name == "console") {
    return PtrType(new ConsoleReporter(output_opts));
  } else if (name == "json") {
    return PtrType(new JSONReporter);
  } else if (name == "csv") {
    return PtrType(new CSVReporter);
  } else {
    std::cerr << "Unexpected format: '" << name << "'\n";
    std::exit(1);
  }
}

}  // end namespace

bool IsZero(double n) {
  return std::abs(n) < std::numeric_limits<double>::epsilon();
}

ConsoleReporter::OutputOptions GetOutputOptions(bool force_no_color) {
  int output_opts = ConsoleReporter::OO_Defaults;
  auto is_benchmark_color = [force_no_color] () -> bool {
    if (force_no_color) {
      return false;
    }
    if (FLAGS_benchmark_color == "auto") {
      return IsColorTerminal();
    }
    return IsTruthyFlagValue(FLAGS_benchmark_color);
  };
  if (is_benchmark_color()) {
    output_opts |= ConsoleReporter::OO_Color;
  } else {
    output_opts &= ~ConsoleReporter::OO_Color;
  }
  if (FLAGS_benchmark_counters_tabular) {
    output_opts |= ConsoleReporter::OO_Tabular;
  } else {
    output_opts &= ~ConsoleReporter::OO_Tabular;
  }
  return static_cast<ConsoleReporter::OutputOptions>(output_opts);
}

}  // end namespace internal

size_t RunSpecifiedBenchmarks() {
  return RunSpecifiedBenchmarks(nullptr, nullptr);
}

size_t RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter) {
  return RunSpecifiedBenchmarks(display_reporter, nullptr);
}

size_t RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter,
                              BenchmarkReporter* file_reporter) {
  std::string spec = FLAGS_benchmark_filter;
  if (spec.empty() || spec == "all")
    spec = ".";  // Regexp that matches all benchmarks

  // Setup the reporters
  std::ofstream output_file;
  std::unique_ptr<BenchmarkReporter> default_display_reporter;
  std::unique_ptr<BenchmarkReporter> default_file_reporter;
  if (!display_reporter) {
    default_display_reporter = internal::CreateReporter(
        FLAGS_benchmark_format, internal::GetOutputOptions());
    display_reporter = default_display_reporter.get();
  }
  auto& Out = display_reporter->GetOutputStream();
  auto& Err = display_reporter->GetErrorStream();

  std::string const& fname = FLAGS_benchmark_out;
  if (fname.empty() && file_reporter) {
    Err << "A custom file reporter was provided but "
           "--benchmark_out=<file> was not specified."
        << std::endl;
    std::exit(1);
  }
  if (!fname.empty()) {
    output_file.open(fname);
    if (!output_file.is_open()) {
      Err << "invalid file name: '" << fname << std::endl;
      std::exit(1);
    }
    if (!file_reporter) {
      default_file_reporter = internal::CreateReporter(
          FLAGS_benchmark_out_format, ConsoleReporter::OO_None);
      file_reporter = default_file_reporter.get();
    }
    file_reporter->SetOutputStream(&output_file);
    file_reporter->SetErrorStream(&output_file);
  }

  std::vector<internal::BenchmarkInstance> benchmarks;
  if (!FindBenchmarksInternal(spec, &benchmarks, &Err)) return 0;

  if (benchmarks.empty()) {
    Err << "Failed to match any benchmarks against regex: " << spec << "\n";
    return 0;
  }

  if (FLAGS_benchmark_list_tests) {
    for (auto const& benchmark : benchmarks) Out << benchmark.name << "\n";
  } else {
    internal::RunBenchmarks(benchmarks, display_reporter, file_reporter);
  }

  return benchmarks.size();
}

void RegisterMemoryManager(MemoryManager* manager) {
  internal::memory_manager = manager;
}

namespace internal {

void PrintUsageAndExit() {
  fprintf(stdout,
          "benchmark"
          " [--benchmark_list_tests={true|false}]\n"
          "          [--benchmark_filter=<regex>]\n"
          "          [--benchmark_min_time=<min_time>]\n"
          "          [--benchmark_repetitions=<num_repetitions>]\n"
          "          [--benchmark_report_aggregates_only={true|false}]\n"
          "          [--benchmark_display_aggregates_only={true|false}]\n"
          "          [--benchmark_format=<console|json|csv>]\n"
          "          [--benchmark_out=<filename>]\n"
          "          [--benchmark_out_format=<json|console|csv>]\n"
          "          [--benchmark_color={auto|true|false}]\n"
          "          [--benchmark_counters_tabular={true|false}]\n"
          "          [--v=<verbosity>]\n");
  exit(0);
}

void ParseCommandLineFlags(int* argc, char** argv) {
  using namespace benchmark;
  BenchmarkReporter::Context::executable_name =
      (argc && *argc > 0) ? argv[0] : "unknown";
  for (int i = 1; i < *argc; ++i) {
    if (ParseBoolFlag(argv[i], "benchmark_list_tests",
                      &FLAGS_benchmark_list_tests) ||
        ParseStringFlag(argv[i], "benchmark_filter", &FLAGS_benchmark_filter) ||
        ParseDoubleFlag(argv[i], "benchmark_min_time",
                        &FLAGS_benchmark_min_time) ||
        ParseInt32Flag(argv[i], "benchmark_repetitions",
                       &FLAGS_benchmark_repetitions) ||
        ParseBoolFlag(argv[i], "benchmark_report_aggregates_only",
                      &FLAGS_benchmark_report_aggregates_only) ||
        ParseBoolFlag(argv[i], "benchmark_display_aggregates_only",
                      &FLAGS_benchmark_display_aggregates_only) ||
        ParseStringFlag(argv[i], "benchmark_format", &FLAGS_benchmark_format) ||
        ParseStringFlag(argv[i], "benchmark_out", &FLAGS_benchmark_out) ||
        ParseStringFlag(argv[i], "benchmark_out_format",
                        &FLAGS_benchmark_out_format) ||
        ParseStringFlag(argv[i], "benchmark_color", &FLAGS_benchmark_color) ||
        // "color_print" is the deprecated name for "benchmark_color".
        // TODO: Remove this.
        ParseStringFlag(argv[i], "color_print", &FLAGS_benchmark_color) ||
        ParseBoolFlag(argv[i], "benchmark_counters_tabular",
                      &FLAGS_benchmark_counters_tabular) ||
        ParseInt32Flag(argv[i], "v", &FLAGS_v)) {
      for (int j = i; j != *argc - 1; ++j) argv[j] = argv[j + 1];

      --(*argc);
      --i;
    } else if (IsFlag(argv[i], "help")) {
      PrintUsageAndExit();
    }
  }
  for (auto const* flag :
       {&FLAGS_benchmark_format, &FLAGS_benchmark_out_format})
    if (*flag != "console" && *flag != "json" && *flag != "csv") {
      PrintUsageAndExit();
    }
  if (FLAGS_benchmark_color.empty()) {
    PrintUsageAndExit();
  }
}

int InitializeStreams() {
  static std::ios_base::Init init;
  return 0;
}

}  // end namespace internal

void Initialize(int* argc, char** argv) {
  internal::ParseCommandLineFlags(argc, argv);
  internal::LogLevel() = FLAGS_v;
}

bool ReportUnrecognizedArguments(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    fprintf(stderr, "%s: error: unrecognized command-line flag: %s\n", argv[0],
            argv[i]);
  }
  return argc > 1;
}

}  // end namespace benchmark
