/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "libhwbinder_benchmark"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include <log/log.h>
#include <utils/StrongPointer.h>

#include <benchmark/benchmark.h>
#include <hidl/Status.h>
#include <hidl/ServiceManagement.h>

#include <android/hardware/tests/libhwbinder/1.0/IBenchmark.h>

// libutils:
using android::OK;
using android::sp;
using android::status_t;

// libhidl:
using android::hardware::defaultServiceManager;
using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;

// Standard library
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;
using std::vector;

// Generated HIDL files
using android::hardware::tests::libhwbinder::V1_0::IBenchmark;

const char gServiceName[] = "android.hardware.tests.libhwbinder.IBenchmark";

static bool startServer() {
    sp<IBenchmark> service = IBenchmark::getService(gServiceName, true);
    status_t status = service->registerAsService(gServiceName);

    if (status != ::android::OK) {
        ALOGE("Failed to register service %s.", gServiceName);
        exit(EXIT_FAILURE);
    }

    return 0;
}

static void BM_sendVec(benchmark::State& state, sp<IBenchmark> service) {
    // Prepare data to IPC
    hidl_vec<uint8_t> data_vec;
    data_vec.resize(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
       data_vec[i] = i % 256;
    }
    // Start running
    while (state.KeepRunning()) {
       service->sendVec(data_vec, [&] (const auto &/*res*/) {
               });
    }
}

static void BM_sendVec_passthrough(benchmark::State& state) {
    // getService automatically retries
    sp<IBenchmark> service = IBenchmark::getService(gServiceName, true /* getStub */);
    if (service == nullptr) {
        state.SkipWithError("Failed to retrieve benchmark service.");
    }
    if (service->isRemote()) {
        state.SkipWithError("Benchmark service is remote.");
    }
    BM_sendVec(state, service);
}

static void BM_sendVec_binderize(benchmark::State& state) {
    // getService automatically retries
    sp<IBenchmark> service = IBenchmark::getService(gServiceName);
    if (service == nullptr) {
        state.SkipWithError("Failed to retrieve benchmark service.");
    }
    if (!service->isRemote()) {
        state.SkipWithError("Unable to fetch remote benchmark service.");
    }
    BM_sendVec(state, service);
}

int main(int argc, char* argv []) {
    android::hardware::details::setTrebleTestingOverride(true);

    enum HwBinderMode {
        kBinderize = 0,
        kPassthrough = 1,
    };
    HwBinderMode mode = HwBinderMode::kBinderize;

    // Parse arguments.
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-m") {
            if (!strcmp(argv[i + 1], "PASSTHROUGH")) {
                mode = HwBinderMode::kPassthrough;
            }
            break;
        }
    }
    if (mode == HwBinderMode::kBinderize) {
        BENCHMARK(BM_sendVec_binderize)->RangeMultiplier(2)->Range(4, 65536);
    } else {
        BENCHMARK(BM_sendVec_passthrough)->RangeMultiplier(2)->Range(4, 65536);
    }

    ::benchmark::Initialize(&argc, argv);

    pid_t pid = fork();
    if (pid == 0) {
        // Child, start benchmarks
        ::benchmark::RunSpecifiedBenchmarks();
    } else {
        startServer();
        while (true) {
            int stat, retval;
            retval = wait(&stat);
            if (retval == -1 && errno == ECHILD) {
                break;
            }
        }
    };
}
