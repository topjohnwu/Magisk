#include "benchmark/benchmark.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

class TestReporter : public benchmark::ConsoleReporter {
 public:
  virtual bool ReportContext(const Context& context) {
    return ConsoleReporter::ReportContext(context);
  };

  virtual void ReportRuns(const std::vector<Run>& report) {
    ++count_;
    ConsoleReporter::ReportRuns(report);
  };

  TestReporter() : count_(0) {}

  virtual ~TestReporter() {}

  size_t GetCount() const { return count_; }

 private:
  mutable size_t count_;
};

}  // end namespace

static void NoPrefix(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(NoPrefix);

static void BM_Foo(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_Foo);

static void BM_Bar(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_Bar);

static void BM_FooBar(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_FooBar);

static void BM_FooBa(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_FooBa);

int main(int argc, char **argv) {
  bool list_only = false;
  for (int i = 0; i < argc; ++i)
    list_only |= std::string(argv[i]).find("--benchmark_list_tests") !=
                 std::string::npos;

  benchmark::Initialize(&argc, argv);

  TestReporter test_reporter;
  const size_t returned_count =
      benchmark::RunSpecifiedBenchmarks(&test_reporter);

  if (argc == 2) {
    // Make sure we ran all of the tests
    std::stringstream ss(argv[1]);
    size_t expected_return;
    ss >> expected_return;

    if (returned_count != expected_return) {
      std::cerr << "ERROR: Expected " << expected_return
                << " tests to match the filter but returned_count = "
                << returned_count << std::endl;
      return -1;
    }

    const size_t expected_reports = list_only ? 0 : expected_return;
    const size_t reports_count = test_reporter.GetCount();
    if (reports_count != expected_reports) {
      std::cerr << "ERROR: Expected " << expected_reports
                << " tests to be run but reported_count = " << reports_count
                << std::endl;
      return -1;
    }
  }

  return 0;
}
