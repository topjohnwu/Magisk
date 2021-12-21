/*
 * Copyright 2020 The Android Open Source Project
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

#include <functional>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/LruCache.h"
#include "utils/StrongPointer.h"

typedef android::LruCache<size_t, size_t> FuzzCache;

static constexpr uint32_t MAX_CACHE_ENTRIES = 800;

class NoopRemovedCallback : public android::OnEntryRemoved<size_t, size_t> {
  public:
    void operator()(size_t&, size_t&) {
        // noop
    }
};

static NoopRemovedCallback callback;

static const std::vector<std::function<void(FuzzedDataProvider*, FuzzCache*)>> operations = {
        [](FuzzedDataProvider*, FuzzCache* cache) -> void { cache->removeOldest(); },
        [](FuzzedDataProvider*, FuzzCache* cache) -> void { cache->peekOldestValue(); },
        [](FuzzedDataProvider*, FuzzCache* cache) -> void { cache->clear(); },
        [](FuzzedDataProvider*, FuzzCache* cache) -> void { cache->size(); },
        [](FuzzedDataProvider*, FuzzCache* cache) -> void {
            android::LruCache<size_t, size_t>::Iterator iter(*cache);
            while (iter.next()) {
                iter.key();
                iter.value();
            }
        },
        [](FuzzedDataProvider* dataProvider, FuzzCache* cache) -> void {
            size_t key = dataProvider->ConsumeIntegral<size_t>();
            size_t val = dataProvider->ConsumeIntegral<size_t>();
            cache->put(key, val);
        },
        [](FuzzedDataProvider* dataProvider, FuzzCache* cache) -> void {
            size_t key = dataProvider->ConsumeIntegral<size_t>();
            cache->get(key);
        },
        [](FuzzedDataProvider* dataProvider, FuzzCache* cache) -> void {
            size_t key = dataProvider->ConsumeIntegral<size_t>();
            cache->remove(key);
        },
        [](FuzzedDataProvider*, FuzzCache* cache) -> void {
            cache->setOnEntryRemovedListener(&callback);
        }};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider dataProvider(data, size);
    FuzzCache cache(MAX_CACHE_ENTRIES);
    while (dataProvider.remaining_bytes() > 0) {
        uint8_t op = dataProvider.ConsumeIntegral<uint8_t>() % operations.size();
        operations[op](&dataProvider, &cache);
    }

    return 0;
}
