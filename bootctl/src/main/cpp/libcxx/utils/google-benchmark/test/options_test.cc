#include "benchmark/benchmark.h"
#include <chrono>
#include <thread>

#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>

void BM_basic(benchmark::State& state) {
  for (auto _ : state) {
  }
}

void BM_basic_slow(benchmark::State& state) {
  std::chrono::milliseconds sleep_duration(state.range(0));
  for (auto _ : state) {
    std::this_thread::sleep_for(
        std::chrono::duration_cast<std::chrono::nanoseconds>(sleep_duration));
  }
}

BENCHMARK(BM_basic);
BENCHMARK(BM_basic)->Arg(42);
BENCHMARK(BM_basic_slow)->Arg(10)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_basic_slow)->Arg(100)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_basic_slow)->Arg(1000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_basic)->Range(1, 8);
BENCHMARK(BM_basic)->RangeMultiplier(2)->Range(1, 8);
BENCHMARK(BM_basic)->DenseRange(10, 15);
BENCHMARK(BM_basic)->Args({42, 42});
BENCHMARK(BM_basic)->Ranges({{64, 512}, {64, 512}});
BENCHMARK(BM_basic)->MinTime(0.7);
BENCHMARK(BM_basic)->UseRealTime();
BENCHMARK(BM_basic)->ThreadRange(2, 4);
BENCHMARK(BM_basic)->ThreadPerCpu();
BENCHMARK(BM_basic)->Repetitions(3);

void CustomArgs(benchmark::internal::Benchmark* b) {
  for (int i = 0; i < 10; ++i) {
    b->Arg(i);
  }
}

BENCHMARK(BM_basic)->Apply(CustomArgs);

void BM_explicit_iteration_count(benchmark::State& state) {
  // Test that benchmarks specified with an explicit iteration count are
  // only run once.
  static bool invoked_before = false;
  assert(!invoked_before);
  invoked_before = true;

  // Test that the requested iteration count is respected.
  assert(state.max_iterations == 42);
  size_t actual_iterations = 0;
  for (auto _ : state)
    ++actual_iterations;
  assert(state.iterations() == state.max_iterations);
  assert(state.iterations() == 42);

}
BENCHMARK(BM_explicit_iteration_count)->Iterations(42);

BENCHMARK_MAIN();
