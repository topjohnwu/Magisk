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

#ifndef BENCHMARK_RUNNER_H_
#define BENCHMARK_RUNNER_H_

#include "benchmark_api_internal.h"
#include "internal_macros.h"

DECLARE_double(benchmark_min_time);

DECLARE_int32(benchmark_repetitions);

DECLARE_bool(benchmark_report_aggregates_only);

DECLARE_bool(benchmark_display_aggregates_only);

namespace benchmark {

namespace internal {

extern MemoryManager* memory_manager;

struct RunResults {
  std::vector<BenchmarkReporter::Run> non_aggregates;
  std::vector<BenchmarkReporter::Run> aggregates_only;

  bool display_report_aggregates_only = false;
  bool file_report_aggregates_only = false;
};

RunResults RunBenchmark(
    const benchmark::internal::BenchmarkInstance& b,
    std::vector<BenchmarkReporter::Run>* complexity_reports);

}  // namespace internal

}  // end namespace benchmark

#endif  // BENCHMARK_RUNNER_H_
