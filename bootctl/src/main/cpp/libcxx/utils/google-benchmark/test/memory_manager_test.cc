#include <memory>

#include "../src/check.h"
#include "benchmark/benchmark.h"
#include "output_test.h"

class TestMemoryManager : public benchmark::MemoryManager {
  void Start() {}
  void Stop(Result* result) {
    result->num_allocs = 42;
    result->max_bytes_used = 42000;
  }
};

void BM_empty(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
  }
}
BENCHMARK(BM_empty);

ADD_CASES(TC_ConsoleOut, {{"^BM_empty %console_report$"}});
ADD_CASES(TC_JSONOut, {{"\"name\": \"BM_empty\",$"},
                       {"\"run_name\": \"BM_empty\",$", MR_Next},
                       {"\"run_type\": \"iteration\",$", MR_Next},
                       {"\"iterations\": %int,$", MR_Next},
                       {"\"real_time\": %float,$", MR_Next},
                       {"\"cpu_time\": %float,$", MR_Next},
                       {"\"time_unit\": \"ns\",$", MR_Next},
                       {"\"allocs_per_iter\": %float,$", MR_Next},
                       {"\"max_bytes_used\": 42000$", MR_Next},
                       {"}", MR_Next}});
ADD_CASES(TC_CSVOut, {{"^\"BM_empty\",%csv_report$"}});


int main(int argc, char *argv[]) {
  std::unique_ptr<benchmark::MemoryManager> mm(new TestMemoryManager());

  benchmark::RegisterMemoryManager(mm.get());
  RunOutputTests(argc, argv);
  benchmark::RegisterMemoryManager(nullptr);
}
