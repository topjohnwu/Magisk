/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "android-base/format.h"

#include <limits>

#include <benchmark/benchmark.h>

#include "android-base/stringprintf.h"

using android::base::StringPrintf;

static void BenchmarkFormatInt(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(fmt::format("{} {} {}", 42, std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max()));
  }
}

BENCHMARK(BenchmarkFormatInt);

static void BenchmarkStringPrintfInt(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(StringPrintf("%d %d %d", 42, std::numeric_limits<int>::min(),
                                          std::numeric_limits<int>::max()));
  }
}

BENCHMARK(BenchmarkStringPrintfInt);

static void BenchmarkFormatFloat(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(fmt::format("{} {} {}", 42.42, std::numeric_limits<float>::min(),
                                         std::numeric_limits<float>::max()));
  }
}

BENCHMARK(BenchmarkFormatFloat);

static void BenchmarkStringPrintfFloat(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(StringPrintf("%f %f %f", 42.42, std::numeric_limits<float>::min(),
                                          std::numeric_limits<float>::max()));
  }
}

BENCHMARK(BenchmarkStringPrintfFloat);

static void BenchmarkFormatStrings(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(fmt::format("{} hello there {}", "hi,", "!!"));
  }
}

BENCHMARK(BenchmarkFormatStrings);

static void BenchmarkStringPrintfStrings(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(StringPrintf("%s hello there %s", "hi,", "!!"));
  }
}

BENCHMARK(BenchmarkStringPrintfStrings);

// Run the benchmark
BENCHMARK_MAIN();
