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
#include <android/hardware/tests/libhwbinder/1.0/IScheduleTest.h>
#include <hidl/LegacySupport.h>
#include <pthread.h>
#include <sys/wait.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include "PerfTest.h"

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(cond)                                                                              \
    do {                                                                                          \
        if (!(cond)) {                                                                            \
            cerr << __func__ << ":" << __LINE__ << " condition:" << #cond << " failed\n" << endl; \
            exit(EXIT_FAILURE);                                                                   \
        }                                                                                         \
    } while (0)

#define REQUIRE(stat)      \
    do {                   \
        int cond = (stat); \
        ASSERT(cond);      \
    } while (0)

using android::hardware::registerPassthroughServiceImplementation;
using android::hardware::tests::libhwbinder::V1_0::IScheduleTest;
using android::sp;
using std::cerr;
using std::cout;
using std::endl;
using std::fstream;
using std::left;
using std::ios;
using std::get;
using std::move;
using std::to_string;
using std::setprecision;
using std::setw;
using std::string;
using std::vector;

static vector<sp<IScheduleTest> > services;

// default arguments
static bool dump_raw_data = false;
static int no_pair = 1;
static int iterations = 100;
static int verbose = 0;
static int is_tracing;
static bool pass_through = false;
// the deadline latency that we are interested in
static uint64_t deadline_us = 2500;

static bool traceIsOn() {
    fstream file;
    file.open(TRACE_PATH "/tracing_on", ios::in);
    char on;
    file >> on;
    file.close();
    return on == '1';
}

static int threadGetPri() {
    sched_param param;
    int policy;
    REQUIRE(!pthread_getschedparam(pthread_self(), &policy, &param));
    return param.sched_priority;
}

static void threadDumpPri(const char* prefix) {
    sched_param param;
    int policy;
    if (!verbose) {
        return;
    }
    cout << "--------------------------------------------------" << endl;
    cout << setw(12) << left << prefix << " pid: " << getpid() << " tid: " << gettid()
         << " cpu: " << sched_getcpu() << endl;
    REQUIRE(!pthread_getschedparam(pthread_self(), &policy, &param));
    string s =
        (policy == SCHED_OTHER)
            ? "SCHED_OTHER"
            : (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : "???";
    cout << setw(12) << left << s << param.sched_priority << endl;
    return;
}

struct ThreadArg {
    void* result;  ///< pointer to PResults
    int target;    ///< the terget service number
};

static void* threadStart(void* p) {
    ThreadArg* priv = (ThreadArg*)p;
    int target = priv->target;
    PResults* presults = (PResults*)priv->result;
    Tick sta, end;

    threadDumpPri("fifo-caller");
    uint32_t call_sta = (threadGetPri() << 16) | sched_getcpu();
    sp<IScheduleTest> service = services[target];
    TICK_NOW(sta);
    uint32_t ret = service->send(verbose, call_sta);
    TICK_NOW(end);
    presults->fifo.addTime(tickDiffNS(sta, end));

    presults->nNotInherent += (ret >> 16) & 0xffff;
    presults->nNotSync += ret & 0xffff;
    return 0;
}

// create a fifo thread to transact and wait it to finished
static void threadTransaction(int target, PResults* presults) {
    ThreadArg thread_arg;
    void* dummy;
    pthread_t thread;
    pthread_attr_t attr;
    sched_param param;
    thread_arg.target = target;
    thread_arg.result = presults;
    REQUIRE(!pthread_attr_init(&attr));
    REQUIRE(!pthread_attr_setschedpolicy(&attr, SCHED_FIFO));
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    REQUIRE(!pthread_attr_setschedparam(&attr, &param));
    REQUIRE(!pthread_create(&thread, &attr, threadStart, &thread_arg));
    REQUIRE(!pthread_join(thread, &dummy));
}

static void serviceFx(const string& serviceName, Pipe p) {
    // Start service.
    if (registerPassthroughServiceImplementation<IScheduleTest>(serviceName) != ::android::OK) {
        cerr << "Failed to register service " << serviceName.c_str() << endl;
        exit(EXIT_FAILURE);
    }
    // tell main I'm init-ed
    p.signal();
    // wait for kill
    p.wait();
    exit(0);
}

static Pipe makeServiceProces(string service_name) {
    auto pipe_pair = Pipe::createPipePair();
    pid_t pid = fork();
    if (pid) {
        // parent
        return move(get<0>(pipe_pair));
    } else {
        threadDumpPri("service");
        // child
        serviceFx(service_name, move(get<1>(pipe_pair)));
        // never get here
        ASSERT(0);
        return move(get<0>(pipe_pair));
    }
}

static void clientFx(int num, int server_count, int iterations, Pipe p) {
    PResults presults;

    presults.fifo.setTracingMode(is_tracing, deadline_us);
    if (dump_raw_data) {
        presults.fifo.setupRawData();
    }

    for (int i = 0; i < server_count; i++) {
        sp<IScheduleTest> service =
            IScheduleTest::getService("hwbinderService" + to_string(i), pass_through);
        ASSERT(service != nullptr);
        if (pass_through) {
            ASSERT(!service->isRemote());
        } else {
            ASSERT(service->isRemote());
        }
        services.push_back(service);
    }
    // tell main I'm init-ed
    p.signal();
    // wait for kick-off
    p.wait();

    // Client for each pair iterates here
    // each iterations contains exactly 2 transactions
    for (int i = 0; i < iterations; i++) {
        Tick sta, end;
        // the target is paired to make it easier to diagnose
        int target = num;

        // 1. transaction by fifo thread
        threadTransaction(target, &presults);
        threadDumpPri("other-caller");

        uint32_t call_sta = (threadGetPri() << 16) | sched_getcpu();
        sp<IScheduleTest> service = services[target];
        // 2. transaction by other thread
        TICK_NOW(sta);
        uint32_t ret = service->send(verbose, call_sta);
        TICK_NOW(end);
        presults.other.addTime(tickDiffNS(sta, end));
        presults.nNotInherent += (ret >> 16) & 0xffff;
        presults.nNotSync += ret & 0xffff;
    }
    // tell main i'm done
    p.signal();

    // wait to send result
    p.wait();
    if (dump_raw_data) {
        cout << "\"fifo_" + to_string(num) + "_data\": ";
        presults.flushRawData();
    }
    cout.flush();
    int sent = p.send(presults);
    ASSERT(sent >= 0);

    // wait for kill
    p.wait();
    exit(0);
}

static Pipe makeClientProcess(int num, int iterations, int no_pair) {
    auto pipe_pair = Pipe::createPipePair();
    pid_t pid = fork();
    if (pid) {
        // parent
        return move(get<0>(pipe_pair));
    } else {
        // child
        threadDumpPri("client");
        clientFx(num, no_pair, iterations, move(get<1>(pipe_pair)));
        // never get here
        ASSERT(0);
        return move(get<0>(pipe_pair));
    }
}

static void waitAll(vector<Pipe>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        v[i].wait();
    }
}

static void signalAll(vector<Pipe>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        v[i].signal();
    }
}

static void help() {
    cout << "usage:" << endl;
    cout << "-i 1              # number of iterations" << endl;
    cout << "-pair 4           # number of process pairs" << endl;
    cout << "-deadline_us 2500 # deadline in us" << endl;
    cout << "-v                # debug" << endl;
    cout << "-raw_data         # dump raw data" << endl;
    cout << "-trace            # halt the trace on a dealine hit" << endl;
    exit(0);
}

// Test:
//
//  libhwbinder_latency -i 1 -v
//  libhwbinder_latency -i 10000 -pair 4
//  atrace --async_start -c sched idle workq binder_driver freq && \
//    libhwbinder_latency -i 10000 -pair 4 -trace
int main(int argc, char** argv) {
    android::hardware::details::setTrebleTestingOverride(true);

    vector<Pipe> client_pipes;
    vector<Pipe> service_pipes;

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-h") {
            help();
        }
        if (string(argv[i]) == "-m") {
            if (!strcmp(argv[i + 1], "PASSTHROUGH")) {
                pass_through = true;
            }
            i++;
            continue;
        }
        if (string(argv[i]) == "-i") {
            iterations = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (string(argv[i]) == "-pair" || string(argv[i]) == "-w") {
            no_pair = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (string(argv[i]) == "-deadline_us") {
            deadline_us = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (string(argv[i]) == "-v") {
            verbose = 1;
        }
        if (string(argv[i]) == "-raw_data") {
            dump_raw_data = true;
        }
        // The -trace argument is used like that:
        //
        // First start trace with atrace command as usual
        // >atrace --async_start sched freq
        //
        // then use the -trace arguments like
        // -trace -deadline_us 2500
        //
        // This makes the program to stop trace once it detects a transaction
        // duration over the deadline. By writing '0' to
        // /sys/kernel/debug/tracing and halt the process. The tracelog is
        // then available on /sys/kernel/debug/trace
        if (string(argv[i]) == "-trace") {
            is_tracing = 1;
        }
    }
    if (!pass_through) {
        // Create services.
        for (int i = 0; i < no_pair; i++) {
            service_pipes.push_back(makeServiceProces("hwbinderService" + to_string(i)));
        }
        // Wait until all services are up.
        waitAll(service_pipes);
    }
    if (is_tracing && !traceIsOn()) {
        cerr << "trace is not running" << endl;
        cerr << "check " << TRACE_PATH "/tracing_on" << endl;
        cerr << "use atrace --async_start first" << endl;
        exit(EXIT_FAILURE);
    }
    threadDumpPri("main");
    cout << "{" << endl;
    cout << "\"cfg\":{\"pair\":" << (no_pair) << ",\"iterations\":" << iterations
         << ",\"deadline_us\":" << deadline_us << ",\"passthrough\":" << pass_through << "},"
         << endl;

    // the main process fork 2 processes for each pairs
    // 1 server + 1 client
    // each has a pipe to communicate with
    for (int i = 0; i < no_pair; i++) {
        client_pipes.push_back(makeClientProcess(i, iterations, no_pair));
    }
    // wait client to init
    waitAll(client_pipes);

    // kick off clients
    signalAll(client_pipes);

    // wait client to finished
    waitAll(client_pipes);

    // collect all results
    PResults total, presults[no_pair];
    for (int i = 0; i < no_pair; i++) {
        client_pipes[i].signal();
        int recvd = client_pipes[i].recv(presults[i]);
        ASSERT(recvd >= 0);
        total = PResults::combine(total, presults[i]);
    }
    cout << "\"ALL\":";
    total.dump();
    for (int i = 0; i < no_pair; i++) {
        cout << "\"P" << i << "\":";
        presults[i].dump();
    }

    if (!pass_through) {
        signalAll(service_pipes);
    }
    int nNotInherent = 0;
    for (int i = 0; i < no_pair; i++) {
        nNotInherent += presults[i].nNotInherent;
    }
    cout << "\"inheritance\": " << (nNotInherent == 0 ? "\"PASS\"" : "\"FAIL\"") << endl;
    cout << "}" << endl;
    // kill all
    signalAll(client_pipes);
    return -nNotInherent;
}
