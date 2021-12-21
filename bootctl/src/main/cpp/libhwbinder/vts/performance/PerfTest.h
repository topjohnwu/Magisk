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
#ifndef HWBINDER_PERF_TEST_H
#define HWBINDER_PERF_TEST_H

#include <unistd.h>
#include <chrono>
#include <list>
#include <tuple>

#define TRACE_PATH "/sys/kernel/debug/tracing"

using std::list;
using std::tuple;

// Pipe is a object used for IPC between parent process and child process.
// This IPC class is widely used in binder/hwbinder tests.
// The common usage is the main process to create the Pipe and forks.
// Both parent and child hold a object. Each recv() on one side requires
// a send() on the other side to unblock.
class Pipe {
   public:
    static tuple<Pipe, Pipe> createPipePair();
    Pipe(Pipe&& rval) noexcept;
    ~Pipe();
    inline void signal() {
        bool val = true;
        send(val);
    }
    inline void wait() {
        bool val = false;
        recv(val);
    }

    // write a data struct
    template <typename T>
    int send(const T& v) {
        return write(fd_write_, &v, sizeof(T));
    }
    // read a data struct
    template <typename T>
    int recv(T& v) {
        return read(fd_read_, &v, sizeof(T));
    }

   private:
    int fd_read_;   // file descriptor to read
    int fd_write_;  // file descriptor to write
    Pipe(int read_fd, int write_fd) : fd_read_{read_fd}, fd_write_{write_fd} {}
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;
    Pipe& operator=(const Pipe&&) = delete;
};

// statistics of latency
// common usage:
//
//  Results r;
//  Tick sta, end;
//  TICK_NOW(sta);
//    ... do something ...
//  TICK_NOW(end);
//  r.addTime(tickDiffNS(sta, end));
//
//  r.dump();
//  r.dumpDistribution();
//
class Results {
   public:
    // enable the deadline miss detection which stops the trace recording after
    // a transaction latency > deadline_us_ is detected.
    void setTracingMode(bool tracing, uint64_t deadline_us) {
        tracing_ = tracing;
        deadline_us_ = deadline_us;
    }
    inline uint64_t getTransactions() const { return transactions_; }
    inline bool missDeadline(uint64_t nano) const { return nano > deadline_us_ * 1000; }
    // Combine two sets of latency data points and update the aggregation info.
    static Results combine(const Results& a, const Results& b);
    // add a new transaction latency record
    void addTime(uint64_t nano);
    // prepare for raw data recording, it may allocate resources which requires
    // a flushRawData() to release
    void setupRawData();
    // dump the raw data and release the resource
    void flushRawData();
    // dump average, best, worst latency in json
    void dump() const;
    // dump latency distribution in json
    void dumpDistribution() const;

   private:
    static const uint32_t kNumBuckets = 128;
    static const uint64_t kMaxTimeBucket = 50ull * 1000000;
    static const uint64_t kTimePerBucket = kMaxTimeBucket / kNumBuckets;
    static constexpr float kTimePerBucketMS = kTimePerBucket / 1.0E6;
    uint64_t best_ = 0xffffffffffffffffULL;  // best transaction latency in ns.
    uint64_t worst_ = 0;                     // worst transaction latency in ns.
    uint64_t transactions_ = 0;              // number of transactions
    uint64_t total_time_ = 0;                // total transaction time
    uint64_t miss_ = 0;                      // number of transactions whose latency > deadline
    uint32_t buckets_[kNumBuckets] = {0};    // statistics for the distribution
    list<uint64_t>* raw_data_ = nullptr;     // list for raw-data
    bool tracing_ = false;                   // halt the trace log on a deadline miss
    bool raw_dump_ = false;                  // record the raw data for the dump after
    uint64_t deadline_us_ = 2500;            // latency deadline in us.
};

// statistics of a process pair
class PResults {
   public:
    static PResults combine(const PResults& a, const PResults& b);
    int nNotInherent = 0;  ///< #transactions that does not inherit priority
    int nNotSync = 0;      ///< #transactions that are not synced
    Results other;         ///< statistics of CFS-other transactions
    Results fifo;          ///< statistics of RT-fifo transactions
    // dump and flush the raw data
    inline void flushRawData() { fifo.flushRawData(); }
    // dump in json
    void dump() const;
};

// Tick keeps timestamp
typedef std::chrono::time_point<std::chrono::high_resolution_clock> Tick;

// get current timestamp as a Tick
static inline Tick tickNow() {
    return std::chrono::high_resolution_clock::now();
}

#define TICK_NOW(_tick)                \
    do {                               \
        asm volatile("" ::: "memory"); \
        _tick = tickNow();             \
        asm volatile("" ::: "memory"); \
    } while (0)

// get nano seconds between sta & end
static inline uint64_t tickDiffNS(Tick& sta, Tick& end) {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(end - sta).count());
}
#endif
