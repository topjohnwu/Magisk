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

#include <benchmark/benchmark.h>
#include <utils/Vector.h>
#include <vector>

void BM_fill_android_vector(benchmark::State& state) {
    android::Vector<char> v;
    while (state.KeepRunning()) {
        v.push('A');
    }
}
BENCHMARK(BM_fill_android_vector);

void BM_fill_std_vector(benchmark::State& state) {
    std::vector<char> v;
    while (state.KeepRunning()) {
        v.push_back('A');
    }
}
BENCHMARK(BM_fill_std_vector);

void BM_prepend_android_vector(benchmark::State& state) {
    android::Vector<char> v;
    while (state.KeepRunning()) {
        v.insertAt('A', 0);
    }
}
BENCHMARK(BM_prepend_android_vector);

void BM_prepend_std_vector(benchmark::State& state) {
    std::vector<char> v;
    while (state.KeepRunning()) {
        v.insert(v.begin(), 'A');
    }
}
BENCHMARK(BM_prepend_std_vector);

BENCHMARK_MAIN();
