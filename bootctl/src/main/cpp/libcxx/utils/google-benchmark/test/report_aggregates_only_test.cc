
#undef NDEBUG
#include <cstdio>
#include <string>

#include "benchmark/benchmark.h"
#include "output_test.h"

// Ok this test is super ugly. We want to check what happens with the file
// reporter in the presence of ReportAggregatesOnly().
// We do not care about console output, the normal tests check that already.

void BM_SummaryRepeat(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_SummaryRepeat)->Repetitions(3)->ReportAggregatesOnly();

int main(int argc, char* argv[]) {
  const std::string output = GetFileReporterOutput(argc, argv);

  if (SubstrCnt(output, "\"name\": \"BM_SummaryRepeat/repeats:3") != 3 ||
      SubstrCnt(output, "\"name\": \"BM_SummaryRepeat/repeats:3_mean\"") != 1 ||
      SubstrCnt(output, "\"name\": \"BM_SummaryRepeat/repeats:3_median\"") !=
          1 ||
      SubstrCnt(output, "\"name\": \"BM_SummaryRepeat/repeats:3_stddev\"") !=
          1) {
    std::cout << "Precondition mismatch. Expected to only find three "
                 "occurrences of \"BM_SummaryRepeat/repeats:3\" substring:\n"
                 "\"name\": \"BM_SummaryRepeat/repeats:3_mean\", "
                 "\"name\": \"BM_SummaryRepeat/repeats:3_median\", "
                 "\"name\": \"BM_SummaryRepeat/repeats:3_stddev\"\nThe entire "
                 "output:\n";
    std::cout << output;
    return 1;
  }

  return 0;
}
