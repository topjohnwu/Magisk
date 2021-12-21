#include <vector>
#include <functional>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "benchmark/benchmark.h"

#include "ContainerBenchmarks.hpp"
#include "GenerateInput.hpp"

using namespace ContainerBenchmarks;

constexpr std::size_t TestNumInputs = 1024;

BENCHMARK_CAPTURE(BM_ConstructIterIter,
  vector_char,
  std::vector<char>{},
  getRandomIntegerInputs<char>)->Arg(TestNumInputs);

BENCHMARK_CAPTURE(BM_ConstructIterIter,
  vector_size_t,
  std::vector<size_t>{},
  getRandomIntegerInputs<size_t>)->Arg(TestNumInputs);

BENCHMARK_CAPTURE(BM_ConstructIterIter,
  vector_string,
  std::vector<std::string>{},
  getRandomStringInputs)->Arg(TestNumInputs);


BENCHMARK_MAIN();
