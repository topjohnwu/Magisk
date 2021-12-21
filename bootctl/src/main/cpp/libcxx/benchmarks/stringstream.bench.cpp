#include "benchmark/benchmark.h"
#include "test_macros.h"

#include <sstream>

TEST_NOINLINE double istream_numbers();

double istream_numbers() {
  const char *a[] = {
    "-6  69 -71  2.4882e-02 -100 101 -2.00005 5000000 -50000000",
    "-25 71   7 -9.3262e+01 -100 101 -2.00005 5000000 -50000000",
    "-14 53  46 -6.7026e-02 -100 101 -2.00005 5000000 -50000000"
  };

  int a1, a2, a3, a4, a5, a6, a7;
  double f1 = 0.0, f2 = 0.0, q = 0.0;
  for (int i=0; i < 3; i++) {
    std::istringstream s(a[i]);
    s >> a1
      >> a2
      >> a3
      >> f1
      >> a4
      >> a5
      >> f2
      >> a6
      >> a7;
    q += (a1 + a2 + a3 + a4 + a5 + a6 + a7 + f1 + f2)/1000000;
  }
  return q;
}

static void BM_Istream_numbers(benchmark::State &state) {
  double i = 0;
  while (state.KeepRunning())
    benchmark::DoNotOptimize(i += istream_numbers());
}

BENCHMARK(BM_Istream_numbers)->RangeMultiplier(2)->Range(1024, 4096);
BENCHMARK_MAIN();
