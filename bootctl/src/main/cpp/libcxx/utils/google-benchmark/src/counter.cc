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

#include "counter.h"

namespace benchmark {
namespace internal {

double Finish(Counter const& c, int64_t iterations, double cpu_time,
              double num_threads) {
  double v = c.value;
  if (c.flags & Counter::kIsRate) {
    v /= cpu_time;
  }
  if (c.flags & Counter::kAvgThreads) {
    v /= num_threads;
  }
  if (c.flags & Counter::kIsIterationInvariant) {
    v *= iterations;
  }
  if (c.flags & Counter::kAvgIterations) {
    v /= iterations;
  }
  return v;
}

void Finish(UserCounters* l, int64_t iterations, double cpu_time, double num_threads) {
  for (auto& c : *l) {
    c.second.value = Finish(c.second, iterations, cpu_time, num_threads);
  }
}

void Increment(UserCounters* l, UserCounters const& r) {
  // add counters present in both or just in *l
  for (auto& c : *l) {
    auto it = r.find(c.first);
    if (it != r.end()) {
      c.second.value = c.second + it->second;
    }
  }
  // add counters present in r, but not in *l
  for (auto const& tc : r) {
    auto it = l->find(tc.first);
    if (it == l->end()) {
      (*l)[tc.first] = tc.second;
    }
  }
}

bool SameNames(UserCounters const& l, UserCounters const& r) {
  if (&l == &r) return true;
  if (l.size() != r.size()) {
    return false;
  }
  for (auto const& c : l) {
    if (r.find(c.first) == r.end()) {
      return false;
    }
  }
  return true;
}

}  // end namespace internal
}  // end namespace benchmark
