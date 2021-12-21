/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <backtrace/BacktraceMap.h>

#include <libunwind.h>

#include "BacktraceLog.h"
#include "UnwindMap.h"

//-------------------------------------------------------------------------
// libunwind has a single shared address space for the current process
// aka local. If multiple maps are created for the current pid, then
// only update the local address space once, and keep a reference count
// of maps using the same map cursor.
//-------------------------------------------------------------------------
UnwindMap::UnwindMap(pid_t pid) : BacktraceMap(pid) {
  unw_map_cursor_clear(&map_cursor_);
}

UnwindMapRemote::UnwindMapRemote(pid_t pid) : UnwindMap(pid) {
}

UnwindMapRemote::~UnwindMapRemote() {
  unw_map_cursor_destroy(&map_cursor_);
  unw_map_cursor_clear(&map_cursor_);
}

bool UnwindMapRemote::GenerateMap() {
  // Use the map_cursor information to construct the BacktraceMap data
  // rather than reparsing /proc/self/maps.
  unw_map_cursor_reset(&map_cursor_);

  unw_map_t unw_map;
  while (unw_map_cursor_get_next(&map_cursor_, &unw_map)) {
    backtrace_map_t map;

    map.start = unw_map.start;
    map.end = unw_map.end;
    map.offset = unw_map.offset;
    map.load_bias = unw_map.load_base;
    map.flags = unw_map.flags;
    map.name = unw_map.path;

    // The maps are in descending order, but we want them in ascending order.
    maps_.push_front(map);
  }

  return true;
}

bool UnwindMapRemote::Build() {
  return (unw_map_cursor_create(&map_cursor_, pid_) == 0) && GenerateMap();
}

UnwindMapLocal::UnwindMapLocal() : UnwindMap(getpid()), map_created_(false) {
  pthread_rwlock_init(&map_lock_, nullptr);
}

UnwindMapLocal::~UnwindMapLocal() {
  if (map_created_) {
    unw_map_local_destroy();
    unw_map_cursor_clear(&map_cursor_);
  }
}

bool UnwindMapLocal::GenerateMap() {
  // Lock so that multiple threads cannot modify the maps data at the
  // same time.
  pthread_rwlock_wrlock(&map_lock_);

  // It's possible for the map to be regenerated while this loop is occurring.
  // If that happens, get the map again, but only try at most three times
  // before giving up.
  bool generated = false;
  for (int i = 0; i < 3; i++) {
    maps_.clear();

    // Save the map data retrieved so we can tell if it changes.
    unw_map_local_cursor_get(&map_cursor_);

    unw_map_t unw_map;
    int ret;
    while ((ret = unw_map_local_cursor_get_next(&map_cursor_, &unw_map)) > 0) {
      backtrace_map_t map;

      map.start = unw_map.start;
      map.end = unw_map.end;
      map.offset = unw_map.offset;
      map.load_bias = unw_map.load_base;
      map.flags = unw_map.flags;
      map.name = unw_map.path;

      free(unw_map.path);

      // The maps are in descending order, but we want them in ascending order.
      maps_.push_front(map);
    }
    // Check to see if the map changed while getting the data.
    if (ret != -UNW_EINVAL) {
      generated = true;
      break;
    }
  }

  pthread_rwlock_unlock(&map_lock_);

  if (!generated) {
    BACK_LOGW("Unable to generate the map.");
  }
  return generated;
}

bool UnwindMapLocal::Build() {
  return (map_created_ = (unw_map_local_create() == 0)) && GenerateMap();;
}

void UnwindMapLocal::FillIn(uint64_t addr, backtrace_map_t* map) {
  BacktraceMap::FillIn(addr, map);
  if (!IsValid(*map)) {
    // Check to see if the underlying map changed and regenerate the map
    // if it did.
    if (unw_map_local_cursor_valid(&map_cursor_) < 0) {
      if (GenerateMap()) {
        BacktraceMap::FillIn(addr, map);
      }
    }
  }
}
