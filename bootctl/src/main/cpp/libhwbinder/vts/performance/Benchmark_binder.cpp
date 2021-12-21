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

#include <benchmark/benchmark.h>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <utils/String16.h>
#include <utils/StrongPointer.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <android/tests/binder/IBenchmark.h>
#include <android/tests/binder/BnBenchmark.h>

// libutils:
using android::OK;
using android::sp;
using android::status_t;
using android::String16;

// libbinder:
using android::getService;
using android::defaultServiceManager;
using android::ProcessState;
using android::binder::Status;

// Standard library
using std::vector;

// Generated AIDL files
using android::tests::binder::BnBenchmark;
using android::tests::binder::IBenchmark;

const char kServiceName[] = "android.tests.binder.IBenchmark";

class BenchmarkServiceAidl : public BnBenchmark {
 public:
    BenchmarkServiceAidl() {}
    virtual ~BenchmarkServiceAidl() = default;

    Status sendVec(const vector<uint8_t>& data, vector<uint8_t>* _aidl_return) {
        *_aidl_return = data;
        return Status::ok();
    }
};

bool startServer() {
    BenchmarkServiceAidl *service = new BenchmarkServiceAidl();
    // Tells the kernel to spawn zero threads, but startThreadPool() below will still spawn one.
    ProcessState::self()->setThreadPoolMaxThreadCount(0);
    defaultServiceManager()->addService(String16(kServiceName),
                                        service);
    ProcessState::self()->startThreadPool();
    return 0;
}

static void BM_sendVec_binder(benchmark::State& state) {
    sp<IBenchmark> service;
    // Prepare data to IPC
    vector<uint8_t> data_vec;
    vector<uint8_t> data_return;
    data_vec.resize(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
       data_vec[i] = i % 256;
    }
    // getService automatically retries
    status_t status = getService(String16(kServiceName), &service);
    if (status != OK) {
        state.SkipWithError("Failed to retrieve benchmark service.");
    }
    // Start running
    while (state.KeepRunning()) {
       service->sendVec(data_vec, &data_return);
    }
}

BENCHMARK(BM_sendVec_binder)->RangeMultiplier(2)->Range(4, 65536);

int main(int argc, char* argv []) {
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
