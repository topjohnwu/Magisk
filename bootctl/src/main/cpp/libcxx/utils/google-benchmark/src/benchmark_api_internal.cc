#include "benchmark_api_internal.h"

namespace benchmark {
namespace internal {

State BenchmarkInstance::Run(
    size_t iters, int thread_id, internal::ThreadTimer* timer,
    internal::ThreadManager* manager) const {
  State st(iters, arg, thread_id, threads, timer, manager);
  benchmark->Run(st);
  return st;
}

}  // internal
}  // benchmark
