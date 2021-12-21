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

#include "PerfTest.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

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

// the ratio that the service is synced on the same cpu beyond
// GOOD_SYNC_MIN is considered as good
#define GOOD_SYNC_MIN (0.6)

// the precision used for cout to dump float
#define DUMP_PRICISION 2

using std::cerr;
using std::cout;
using std::endl;
using std::left;
using std::ios;
using std::min;
using std::max;
using std::to_string;
using std::setprecision;
using std::setw;
using std::ofstream;
using std::make_tuple;

tuple<Pipe, Pipe> Pipe::createPipePair() {
    int a[2];
    int b[2];

    int error1 = pipe(a);
    int error2 = pipe(b);
    ASSERT(error1 >= 0);
    ASSERT(error2 >= 0);

    return make_tuple(Pipe(a[0], b[1]), Pipe(b[0], a[1]));
}

Pipe::Pipe(Pipe&& rval) noexcept {
    fd_read_ = rval.fd_read_;
    fd_write_ = rval.fd_write_;
    rval.fd_read_ = 0;
    rval.fd_write_ = 0;
}

Pipe::~Pipe() {
    if (fd_read_) {
        close(fd_read_);
    }
    if (fd_write_) {
        close(fd_write_);
    }
}

Results Results::combine(const Results& a, const Results& b) {
    Results ret;
    for (uint32_t i = 0; i < kNumBuckets; i++) {
        ret.buckets_[i] = a.buckets_[i] + b.buckets_[i];
    }
    ret.worst_ = max(a.worst_, b.worst_);
    ret.best_ = min(a.best_, b.best_);
    ret.transactions_ = a.transactions_ + b.transactions_;
    ret.miss_ = a.miss_ + b.miss_;
    ret.total_time_ = a.total_time_ + b.total_time_;
    return ret;
}

static void traceStop() {
    ofstream file;
    file.open(TRACE_PATH "/tracing_on", ios::out | ios::trunc);
    file << '0' << endl;
    file.close();
}

void Results::addTime(uint64_t nano) {
    buckets_[min(nano, kMaxTimeBucket - 1) / kTimePerBucket] += 1;
    best_ = min(nano, best_);
    worst_ = max(nano, worst_);
    if (raw_dump_) {
        raw_data_->push_back(nano);
    }
    transactions_ += 1;
    total_time_ += nano;
    if (missDeadline(nano)) {
        miss_++;
        if (tracing_) {
            // There might be multiple process pair running the test concurrently
            // each may execute following statements and only the first one actually
            // stop the trace and any traceStop() after then has no effect.
            traceStop();
            cerr << endl;
            cerr << "deadline triggered: halt & stop trace" << endl;
            cerr << "log:" TRACE_PATH "/trace" << endl;
            cerr << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void Results::setupRawData() {
    raw_dump_ = true;
    if (raw_data_ == nullptr) {
        raw_data_ = new list<uint64_t>;
    } else {
        raw_data_->clear();
    }
}

void Results::flushRawData() {
    if (raw_dump_) {
        bool first = true;
        cout << "[";
        for (auto nano : *raw_data_) {
            cout << (first ? "" : ",") << to_string(nano);
            first = false;
        }
        cout << "]," << endl;
        delete raw_data_;
        raw_data_ = nullptr;
    }
}

void Results::dump() const {
    double best = (double)best_ / 1.0E6;
    double worst = (double)worst_ / 1.0E6;
    double average = (double)total_time_ / transactions_ / 1.0E6;
    int W = DUMP_PRICISION + 2;
    cout << std::setprecision(DUMP_PRICISION) << "{ \"avg\":" << setw(W) << left << average
         << ", \"wst\":" << setw(W) << left << worst << ", \"bst\":" << setw(W) << left << best
         << ", \"miss\":" << left << miss_ << ", \"meetR\":" << setprecision(DUMP_PRICISION + 3)
         << left << (1.0 - (double)miss_ / transactions_) << "}";
}

void Results::dumpDistribution() const {
    uint64_t cur_total = 0;
    cout << "{ ";
    cout << std::setprecision(DUMP_PRICISION + 3);
    for (uint32_t i = 0; i < kNumBuckets; i++) {
        float cur_time = kTimePerBucketMS * i + 0.5f * kTimePerBucketMS;
        float accumulation = cur_total + buckets_[i];
        if ((cur_total < 0.5f * transactions_) && (accumulation >= 0.5f * transactions_)) {
            cout << "\"p50\":" << cur_time << ", ";
        }
        if ((cur_total < 0.9f * transactions_) && (accumulation >= 0.9f * transactions_)) {
            cout << "\"p90\":" << cur_time << ", ";
        }
        if ((cur_total < 0.95f * transactions_) && (accumulation >= 0.95f * transactions_)) {
            cout << "\"p95\":" << cur_time << ", ";
        }
        if ((cur_total < 0.99f * transactions_) && (accumulation >= 0.99f * transactions_)) {
            cout << "\"p99\": " << cur_time;
        }
        cur_total += buckets_[i];
    }
    cout << "}";
}

PResults PResults::combine(const PResults& a, const PResults& b) {
    PResults ret;
    ret.nNotInherent = a.nNotInherent + b.nNotInherent;
    ret.nNotSync = a.nNotSync + b.nNotSync;
    ret.other = Results::combine(a.other, b.other);
    ret.fifo = Results::combine(a.fifo, b.fifo);
    return ret;
}

void PResults::dump() const {
    int no_trans = other.getTransactions() + fifo.getTransactions();
    double sync_ratio = (1.0 - (double)nNotSync / no_trans);
    cout << "{\"SYNC\":\"" << ((sync_ratio > GOOD_SYNC_MIN) ? "GOOD" : "POOR") << "\","
         << "\"S\":" << (no_trans - nNotSync) << ",\"I\":" << no_trans << ","
         << "\"R\":" << sync_ratio << "," << endl;
    cout << "  \"other_ms\":";
    other.dump();
    cout << "," << endl;
    cout << "  \"fifo_ms\": ";
    fifo.dump();
    cout << "," << endl;
    cout << "  \"otherdis\":";
    other.dumpDistribution();
    cout << "," << endl;
    cout << "  \"fifodis\": ";
    fifo.dumpDistribution();
    cout << endl;
    cout << "}," << endl;
}
