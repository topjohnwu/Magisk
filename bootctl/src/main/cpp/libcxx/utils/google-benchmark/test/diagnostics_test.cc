// Testing:
//   State::PauseTiming()
//   State::ResumeTiming()
// Test that CHECK's within these function diagnose when they are called
// outside of the KeepRunning() loop.
//
// NOTE: Users should NOT include or use src/check.h. This is only done in
// order to test library internals.

#include <cstdlib>
#include <stdexcept>

#include "../src/check.h"
#include "benchmark/benchmark.h"

#if defined(__GNUC__) && !defined(__EXCEPTIONS)
#define TEST_HAS_NO_EXCEPTIONS
#endif

void TestHandler() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  throw std::logic_error("");
#else
  std::abort();
#endif
}

void try_invalid_pause_resume(benchmark::State& state) {
#if !defined(TEST_BENCHMARK_LIBRARY_HAS_NO_ASSERTIONS) && !defined(TEST_HAS_NO_EXCEPTIONS)
  try {
    state.PauseTiming();
    std::abort();
  } catch (std::logic_error const&) {
  }
  try {
    state.ResumeTiming();
    std::abort();
  } catch (std::logic_error const&) {
  }
#else
  (void)state;  // avoid unused warning
#endif
}

void BM_diagnostic_test(benchmark::State& state) {
  static bool called_once = false;

  if (called_once == false) try_invalid_pause_resume(state);

  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
  }

  if (called_once == false) try_invalid_pause_resume(state);

  called_once = true;
}
BENCHMARK(BM_diagnostic_test);


void BM_diagnostic_test_keep_running(benchmark::State& state) {
  static bool called_once = false;

  if (called_once == false) try_invalid_pause_resume(state);

  while(state.KeepRunning()) {
    benchmark::DoNotOptimize(state.iterations());
  }

  if (called_once == false) try_invalid_pause_resume(state);

  called_once = true;
}
BENCHMARK(BM_diagnostic_test_keep_running);

int main(int argc, char* argv[]) {
  benchmark::internal::GetAbortHandler() = &TestHandler;
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
