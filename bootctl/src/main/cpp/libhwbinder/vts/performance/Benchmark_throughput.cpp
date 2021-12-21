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
#define LOG_TAG "HwbinderThroughputTest"

#include <unistd.h>
#include <sys/wait.h>

#include <cstring>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <log/log.h>

#include <android/hardware/tests/libhwbinder/1.0/IBenchmark.h>
#include <hidl/HidlSupport.h>
#include <hidl/ServiceManagement.h>

using namespace std;
using namespace android;
using namespace android::hardware;

// Generated HIDL files
using android::hardware::tests::libhwbinder::V1_0::IBenchmark;

#define ASSERT_TRUE(cond) \
do { \
    if (!(cond)) {\
       cerr << __func__ << ":" << __LINE__ << " condition:" << #cond << " failed\n" << endl; \
       exit(EXIT_FAILURE); \
    } \
} while (0)

class Pipe {
    int m_readFd;
    int m_writeFd;
    Pipe(int readFd, int writeFd)
            : m_readFd{readFd}, m_writeFd{writeFd} {
    }
    Pipe(const Pipe &) = delete;
    Pipe& operator=(const Pipe &) = delete;
    Pipe& operator=(const Pipe &&) = delete;
 public:
    Pipe(Pipe&& rval) noexcept {
        m_readFd = rval.m_readFd;
        m_writeFd = rval.m_writeFd;
        rval.m_readFd = 0;
        rval.m_writeFd = 0;
    }
    ~Pipe() {
        if (m_readFd)
            close(m_readFd);
        if (m_writeFd)
            close(m_writeFd);
    }
    void signal() {
        bool val = true;
        int error = write(m_writeFd, &val, sizeof(val));
        ASSERT_TRUE(error >= 0);
    }
    void wait() {
        bool val = false;
        int error = read(m_readFd, &val, sizeof(val));
        ASSERT_TRUE(error >= 0);
    }
    template<typename T> void send(const T& v) {
        int error = write(m_writeFd, &v, sizeof(T));
        ASSERT_TRUE(error >= 0);
    }
    template<typename T> void recv(T& v) {
        int error = read(m_readFd, &v, sizeof(T));
        ASSERT_TRUE(error >= 0);
    }
    static tuple<Pipe, Pipe> createPipePair() {
        int a[2];
        int b[2];

        int error1 = pipe(a);
        int error2 = pipe(b);
        ASSERT_TRUE(error1 >= 0);
        ASSERT_TRUE(error2 >= 0);

        return make_tuple(Pipe(a[0], b[1]), Pipe(b[0], a[1]));
    }
};

static const uint32_t num_buckets = 128;
static const uint64_t max_time_bucket = 50ull * 1000000;
static const uint64_t time_per_bucket = max_time_bucket / num_buckets;
static constexpr float time_per_bucket_ms = time_per_bucket / 1.0E6;

struct ProcResults {
    uint64_t m_best = max_time_bucket;
    uint64_t m_worst = 0;
    uint32_t m_buckets[num_buckets] = {0};
    uint64_t m_transactions = 0;
    uint64_t m_total_time = 0;

    // Add a new latency data point and update the aggregation info
    // e.g. best/worst/total_time.
    void add_time(uint64_t time) {
        m_buckets[min(time, max_time_bucket - 1) / time_per_bucket] += 1;
        m_best = min(time, m_best);
        m_worst = max(time, m_worst);
        m_transactions += 1;
        m_total_time += time;
    }
    // Combine two sets of latency data points and update the aggregation info.
    static ProcResults combine(const ProcResults& a, const ProcResults& b) {
        ProcResults ret;
        for (uint32_t i = 0; i < num_buckets; i++) {
            ret.m_buckets[i] = a.m_buckets[i] + b.m_buckets[i];
        }
        ret.m_worst = max(a.m_worst, b.m_worst);
        ret.m_best = min(a.m_best, b.m_best);
        ret.m_transactions = a.m_transactions + b.m_transactions;
        ret.m_total_time = a.m_total_time + b.m_total_time;
        return ret;
    }
    // Calculate and report the final aggregated results.
    void dump() {
        double best = (double) m_best / 1.0E6;
        double worst = (double) m_worst / 1.0E6;
        double average = (double) m_total_time / m_transactions / 1.0E6;
        cout << "average:"
             << average
             << "ms worst:"
             << worst
             << "ms best:"
             << best
             << "ms"
             << endl;

        uint64_t cur_total = 0;
        for (uint32_t i = 0; i < num_buckets; i++) {
            float cur_time = time_per_bucket_ms * i + 0.5f * time_per_bucket_ms;
            if ((cur_total < 0.5f * m_transactions)
                && (cur_total + m_buckets[i] >= 0.5f * m_transactions)) {
                cout << "50%: " << cur_time << " ";
            }
            if ((cur_total < 0.9f * m_transactions)
                && (cur_total + m_buckets[i] >= 0.9f * m_transactions)) {
                cout << "90%: " << cur_time << " ";
            }
            if ((cur_total < 0.95f * m_transactions)
                && (cur_total + m_buckets[i] >= 0.95f * m_transactions)) {
                cout << "95%: " << cur_time << " ";
            }
            if ((cur_total < 0.99f * m_transactions)
                && (cur_total + m_buckets[i] >= 0.99f * m_transactions)) {
                cout << "99%: " << cur_time << " ";
            }
            cur_total += m_buckets[i];
        }
        cout << endl;

    }
};

string generateServiceName(int num) {
    string serviceName = "hwbinderService" + to_string(num);
    return serviceName;
}

void service_fx(const string &serviceName, Pipe p) {
    // Start service.
    sp<IBenchmark> server = IBenchmark::getService(serviceName, true);
    ALOGD("Registering %s", serviceName.c_str());
    status_t status = server->registerAsService(serviceName);
    if (status != ::android::OK) {
        ALOGE("Failed to register service %s", serviceName.c_str());
        exit(EXIT_FAILURE);
    }

    ALOGD("Starting %s", serviceName.c_str());

    // Signal service started to master and wait to exit.
    p.signal();
    p.wait();
    exit(EXIT_SUCCESS);
}

void worker_fx(
        int num,
        int iterations,
        int service_count,
        bool get_stub,
        Pipe p) {
    srand(num);
    p.signal();
    p.wait();

    // Get references to test services.
    vector<sp<IBenchmark>> workers;

    for (int i = 0; i < service_count; i++) {
        sp<IBenchmark> service = IBenchmark::getService(
                generateServiceName(i), get_stub);
        ASSERT_TRUE(service != NULL);
        if (get_stub) {
            ASSERT_TRUE(!service->isRemote());
        } else {
            ASSERT_TRUE(service->isRemote());
        }
        workers.push_back(service);
    }

    ProcResults results;
    chrono::time_point<chrono::high_resolution_clock> start, end;
    // Prepare data to IPC
    hidl_vec<uint8_t> data_vec;
    data_vec.resize(16);
    for (size_t i = 0; i < data_vec.size(); i++) {
        data_vec[i] = i;
    }
    // Run the benchmark.
    for (int i = 0; i < iterations; i++) {
        // Randomly pick a service.
        int target = rand() % service_count;

        start = chrono::high_resolution_clock::now();
        Return<void> ret = workers[target]->sendVec(data_vec, [&](const auto &) {});
        if (!ret.isOk()) {
            cout << "thread " << num << " failed status: "
                << ret.description() << endl;
            exit(EXIT_FAILURE);
        }
        end = chrono::high_resolution_clock::now();

        uint64_t cur_time = uint64_t(
               chrono::duration_cast<chrono::nanoseconds>(end - start).count());
        results.add_time(cur_time);
    }
    // Signal completion to master and wait.
    p.signal();
    p.wait();

    // Send results to master and wait for go to exit.
    p.send(results);
    p.wait();

    exit (EXIT_SUCCESS);
}

Pipe make_service(string service_name) {
    auto pipe_pair = Pipe::createPipePair();
    pid_t pid = fork();
    if (pid) {
        /* parent */
        return move(get<0>(pipe_pair));
    } else {
        /* child */
        service_fx(service_name, move(get<1>(pipe_pair)));
        /* never get here */
        return move(get<0>(pipe_pair));
    }
}

Pipe make_worker(int num, int iterations, int service_count, bool get_stub) {
    auto pipe_pair = Pipe::createPipePair();
    pid_t pid = fork();
    if (pid) {
        /* parent */
        return move(get<0>(pipe_pair));
    } else {
        /* child */
        worker_fx(num, iterations, service_count, get_stub,
                  move(get<1>(pipe_pair)));
        /* never get here */
        return move(get<0>(pipe_pair));
    }
}

void wait_all(vector<Pipe>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        v[i].wait();
    }
}

void signal_all(vector<Pipe>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        v[i].signal();
    }
}

int main(int argc, char *argv[]) {
    android::hardware::details::setTrebleTestingOverride(true);

    enum HwBinderMode {
        kBinderize = 0,
        kPassthrough = 1,
    };
    HwBinderMode mode = HwBinderMode::kBinderize;

    // Num of workers.
    int workers = 2;
    // Num of services.
    int services = -1;
    int iterations = 10000;

    vector<Pipe> worker_pipes;
    vector<Pipe> service_pipes;

    // Parse arguments.
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-m") {
            if (!strcmp(argv[i + 1], "PASSTHROUGH")) {
                mode = HwBinderMode::kPassthrough;
            }
            i++;
            continue;
        }
        if (string(argv[i]) == "-w") {
            workers = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (string(argv[i]) == "-i") {
            iterations = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (string(argv[i]) == "-s") {
            services = atoi(argv[i + 1]);
            i++;
            continue;
        }
    }
    // If service number is not provided, set it the same as the worker number.
    if (services == -1) {
        services = workers;
    }
    if (mode == HwBinderMode::kBinderize) {
        // Create services.
        vector<pid_t> pIds;
        for (int i = 0; i < services; i++) {
            string serviceName = generateServiceName(i);
            cout << "creating service: " << serviceName << endl;
            service_pipes.push_back(make_service(serviceName));
        }
        // Wait until all services are up.
        wait_all(service_pipes);
    }

    // Create workers (test clients).
    bool get_stub = mode == HwBinderMode::kBinderize ? false : true;
    for (int i = 0; i < workers; i++) {
        worker_pipes.push_back(make_worker(i, iterations, services, get_stub));
    }
    // Wait untill all workers are ready.
    wait_all(worker_pipes);

    // Run the workers and wait for completion.
    chrono::time_point<chrono::high_resolution_clock> start, end;
    cout << "waiting for workers to complete" << endl;
    start = chrono::high_resolution_clock::now();
    signal_all(worker_pipes);
    wait_all(worker_pipes);
    end = chrono::high_resolution_clock::now();

    // Calculate overall throughput.
    double iterations_per_sec = double(iterations * workers)
        / (chrono::duration_cast < chrono::nanoseconds
            > (end - start).count() / 1.0E9);
    cout << "iterations per sec: " << iterations_per_sec << endl;

    // Collect all results from the workers.
    cout << "collecting results" << endl;
    signal_all(worker_pipes);
    ProcResults tot_results;
    for (int i = 0; i < workers; i++) {
        ProcResults tmp_results;
        worker_pipes[i].recv(tmp_results);
        tot_results = ProcResults::combine(tot_results, tmp_results);
    }
    tot_results.dump();

    if (mode == HwBinderMode::kBinderize) {
        // Kill all the services.
        cout << "killing services" << endl;
        signal_all(service_pipes);
        for (int i = 0; i < services; i++) {
            int status;
            wait(&status);
            if (status != 0) {
                cout << "nonzero child status" << status << endl;
            }
        }
    }
    // Kill all the workers.
    cout << "killing workers" << endl;
    signal_all(worker_pipes);
    for (int i = 0; i < workers; i++) {
        int status;
        wait(&status);
        if (status != 0) {
            cout << "nonzero child status" << status << endl;
        }
    }
    return 0;
}
