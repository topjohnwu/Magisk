/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <cutils/hashmap.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct Entry Entry;
struct Entry {
    void* key;
    int hash;
    void* value;
    Entry* next;
};

struct Hashmap {
    Entry** buckets;
    size_t bucketCount;
    int (*hash)(void* key);
    bool (*equals)(void* keyA, void* keyB);
    pthread_mutex_t lock;
    size_t size;
};

Hashmap* hashmapCreate(size_t initialCapacity,
        int (*hash)(void* key), bool (*equals)(void* keyA, void* keyB)) {
    assert(hash != NULL);
    assert(equals != NULL);

    Hashmap* map = static_cast<Hashmap*>(malloc(sizeof(Hashmap)));
    if (map == NULL) {
        return NULL;
    }

    // 0.75 load factor.
    size_t minimumBucketCount = initialCapacity * 4 / 3;
    map->bucketCount = 1;
    while (map->bucketCount <= minimumBucketCount) {
        // Bucket count must be power of 2.
        map->bucketCount <<= 1;
    }

    map->buckets = static_cast<Entry**>(calloc(map->bucketCount, sizeof(Entry*)));
    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }

    map->size = 0;

    map->hash = hash;
    map->equals = equals;

    pthread_mutex_init(&map->lock, nullptr);

    return map;
}

/**
 * Hashes the given key.
 */
#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
static inline int hashKey(Hashmap* map, void* key) {
    int h = map->hash(key);

    // We apply this secondary hashing discovered by Doug Lea to defend
    // against bad hashes.
    h += ~(h << 9);
    h ^= (((unsigned int) h) >> 14);
    h += (h << 4);
    h ^= (((unsigned int) h) >> 10);

    return h;
}

static inline size_t calculateIndex(size_t bucketCount, int hash) {
    return ((size_t) hash) & (bucketCount - 1);
}

static void expandIfNecessary(Hashmap* map) {
    // If the load factor exceeds 0.75...
    if (map->size > (map->bucketCount * 3 / 4)) {
        // Start off with a 0.33 load factor.
        size_t newBucketCount = map->bucketCount << 1;
        Entry** newBuckets = static_cast<Entry**>(calloc(newBucketCount, sizeof(Entry*)));
        if (newBuckets == NULL) {
            // Abort expansion.
            return;
        }

        // Move over existing entries.
        size_t i;
        for (i = 0; i < map->bucketCount; i++) {
            Entry* entry = map->buckets[i];
            while (entry != NULL) {
                Entry* next = entry->next;
                size_t index = calculateIndex(newBucketCount, entry->hash);
                entry->next = newBuckets[index];
                newBuckets[index] = entry;
                entry = next;
            }
        }

        // Copy over internals.
        free(map->buckets);
        map->buckets = newBuckets;
        map->bucketCount = newBucketCount;
    }
}

void hashmapLock(Hashmap* map) {
    pthread_mutex_lock(&map->lock);
}

void hashmapUnlock(Hashmap* map) {
    pthread_mutex_unlock(&map->lock);
}

void hashmapFree(Hashmap* map) {
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    pthread_mutex_destroy(&map->lock);
    free(map);
}

#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
/* FIXME: relies on signed integer overflow, which is undefined behavior */
int hashmapHash(void* key, size_t keySize) {
    int h = keySize;
    char* data = (char*) key;
    size_t i;
    for (i = 0; i < keySize; i++) {
        h = h * 31 + *data;
        data++;
    }
    return h;
}

static Entry* createEntry(void* key, int hash, void* value) {
    Entry* entry = static_cast<Entry*>(malloc(sizeof(Entry)));
    if (entry == NULL) {
        return NULL;
    }
    entry->key = key;
    entry->hash = hash;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static inline bool equalKeys(void* keyA, int hashA, void* keyB, int hashB,
        bool (*equals)(void*, void*)) {
    if (keyA == keyB) {
        return true;
    }
    if (hashA != hashB) {
        return false;
    }
    return equals(keyA, keyB);
}

void* hashmapPut(Hashmap* map, void* key, void* value) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry** p = &(map->buckets[index]);
    while (true) {
        Entry* current = *p;

        // Add a new entry.
        if (current == NULL) {
            *p = createEntry(key, hash, value);
            if (*p == NULL) {
                errno = ENOMEM;
                return NULL;
            }
            map->size++;
            expandIfNecessary(map);
            return NULL;
        }

        // Replace existing entry.
        if (equalKeys(current->key, current->hash, key, hash, map->equals)) {
            void* oldValue = current->value;
            current->value = value;
            return oldValue;
        }

        // Move to next entry.
        p = &current->next;
    }
}

void* hashmapGet(Hashmap* map, void* key) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (equalKeys(entry->key, entry->hash, key, hash, map->equals)) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

void* hashmapRemove(Hashmap* map, void* key) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    // Pointer to the current entry.
    Entry** p = &(map->buckets[index]);
    Entry* current;
    while ((current = *p) != NULL) {
        if (equalKeys(current->key, current->hash, key, hash, map->equals)) {
            void* value = current->value;
            *p = current->next;
            free(current);
            map->size--;
            return value;
        }

        p = &current->next;
    }

    return NULL;
}

void hashmapForEach(Hashmap* map, bool (*callback)(void* key, void* value, void* context),
                    void* context) {
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry *next = entry->next;
            if (!callback(entry->key, entry->value, context)) {
                return;
            }
            entry = next;
        }
    }
}
