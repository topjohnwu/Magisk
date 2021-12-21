/*
** Copyright 2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <log/log_properties.h>

#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

#include <android-base/macros.h>

#include <private/android_logger.h>

#include "logger_write.h"

#ifdef __ANDROID__
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

static pthread_mutex_t lock_loggable = PTHREAD_MUTEX_INITIALIZER;

static bool trylock() {
  /*
   * If we trigger a signal handler in the middle of locked activity and the
   * signal handler logs a message, we could get into a deadlock state.
   */
  /*
   *  Any contention, and we can turn around and use the non-cached method
   * in less time than the system call associated with a mutex to deal with
   * the contention.
   */
  return pthread_mutex_trylock(&lock_loggable) == 0;
}

static void unlock() {
  pthread_mutex_unlock(&lock_loggable);
}

struct cache {
  const prop_info* pinfo;
  uint32_t serial;
};

struct cache_char {
  struct cache cache;
  unsigned char c;
};

static int check_cache(struct cache* cache) {
  return cache->pinfo && __system_property_serial(cache->pinfo) != cache->serial;
}

#define BOOLEAN_TRUE 0xFF
#define BOOLEAN_FALSE 0xFE

static void refresh_cache(struct cache_char* cache, const char* key) {
  char buf[PROP_VALUE_MAX];

  if (!cache->cache.pinfo) {
    cache->cache.pinfo = __system_property_find(key);
    if (!cache->cache.pinfo) {
      return;
    }
  }
  cache->cache.serial = __system_property_serial(cache->cache.pinfo);
  __system_property_read(cache->cache.pinfo, 0, buf);
  switch (buf[0]) {
    case 't':
    case 'T':
      cache->c = strcasecmp(buf + 1, "rue") ? buf[0] : BOOLEAN_TRUE;
      break;
    case 'f':
    case 'F':
      cache->c = strcasecmp(buf + 1, "alse") ? buf[0] : BOOLEAN_FALSE;
      break;
    default:
      cache->c = buf[0];
  }
}

static int __android_log_level(const char* tag, size_t tag_len) {
  if (tag == nullptr || tag_len == 0) {
    auto& tag_string = GetDefaultTag();
    tag = tag_string.c_str();
    tag_len = tag_string.size();
  }

  /*
   * Single layer cache of four properties. Priorities are:
   *    log.tag.<tag>
   *    persist.log.tag.<tag>
   *    log.tag
   *    persist.log.tag
   * Where the missing tag matches all tags and becomes the
   * system global default. We do not support ro.log.tag* .
   */
  static std::string* last_tag = new std::string;
  static uint32_t global_serial;
  uint32_t current_global_serial;
  static cache_char tag_cache[2];
  static cache_char global_cache[2];

  static const char* log_namespace = "persist.log.tag.";
  char key[strlen(log_namespace) + tag_len + 1];
  strcpy(key, log_namespace);

  bool locked = trylock();
  bool change_detected, global_change_detected;
  global_change_detected = change_detected = !locked;

  char c = 0;
  if (locked) {
    // Check all known serial numbers for changes.
    for (size_t i = 0; i < arraysize(tag_cache); ++i) {
      if (check_cache(&tag_cache[i].cache)) {
        change_detected = true;
      }
    }
    for (size_t i = 0; i < arraysize(global_cache); ++i) {
      if (check_cache(&global_cache[i].cache)) {
        global_change_detected = true;
      }
    }

    current_global_serial = __system_property_area_serial();
    if (current_global_serial != global_serial) {
      global_change_detected = change_detected = true;
    }
  }

  if (tag_len != 0) {
    bool local_change_detected = change_detected;
    if (locked) {
      // compare() rather than == because tag isn't guaranteed 0-terminated.
      if (last_tag->compare(0, last_tag->size(), tag, tag_len) != 0) {
        // Invalidate log.tag.<tag> cache.
        for (size_t i = 0; i < arraysize(tag_cache); ++i) {
          tag_cache[i].cache.pinfo = NULL;
          tag_cache[i].c = '\0';
        }
        last_tag->assign(tag, tag_len);
        local_change_detected = true;
      }
    }
    *stpncpy(key + strlen(log_namespace), tag, tag_len) = '\0';

    for (size_t i = 0; i < arraysize(tag_cache); ++i) {
      cache_char* cache = &tag_cache[i];
      cache_char temp_cache;

      if (!locked) {
        temp_cache.cache.pinfo = NULL;
        temp_cache.c = '\0';
        cache = &temp_cache;
      }
      if (local_change_detected) {
        refresh_cache(cache, i == 0 ? key : key + strlen("persist."));
      }

      if (cache->c) {
        c = cache->c;
        break;
      }
    }
  }

  switch (toupper(c)) { /* if invalid, resort to global */
    case 'V':
    case 'D':
    case 'I':
    case 'W':
    case 'E':
    case 'F': /* Not officially supported */
    case 'A':
    case 'S':
    case BOOLEAN_FALSE: /* Not officially supported */
      break;
    default:
      /* clear '.' after log.tag */
      key[strlen(log_namespace) - 1] = '\0';

      for (size_t i = 0; i < arraysize(global_cache); ++i) {
        cache_char* cache = &global_cache[i];
        cache_char temp_cache;

        if (!locked) {
          temp_cache = *cache;
          if (temp_cache.cache.pinfo != cache->cache.pinfo) {  // check atomic
            temp_cache.cache.pinfo = NULL;
            temp_cache.c = '\0';
          }
          cache = &temp_cache;
        }
        if (global_change_detected) {
          refresh_cache(cache, i == 0 ? key : key + strlen("persist."));
        }

        if (cache->c) {
          c = cache->c;
          break;
        }
      }
      break;
  }

  if (locked) {
    global_serial = current_global_serial;
    unlock();
  }

  switch (toupper(c)) {
    /* clang-format off */
    case 'V': return ANDROID_LOG_VERBOSE;
    case 'D': return ANDROID_LOG_DEBUG;
    case 'I': return ANDROID_LOG_INFO;
    case 'W': return ANDROID_LOG_WARN;
    case 'E': return ANDROID_LOG_ERROR;
    case 'F': /* FALLTHRU */ /* Not officially supported */
    case 'A': return ANDROID_LOG_FATAL;
    case BOOLEAN_FALSE: /* FALLTHRU */ /* Not Officially supported */
    case 'S': return ANDROID_LOG_SILENT;
      /* clang-format on */
  }
  return -1;
}

int __android_log_is_loggable_len(int prio, const char* tag, size_t len, int default_prio) {
  int minimum_log_priority = __android_log_get_minimum_priority();
  int property_log_level = __android_log_level(tag, len);

  if (property_log_level >= 0 && minimum_log_priority != ANDROID_LOG_DEFAULT) {
    return prio >= std::min(property_log_level, minimum_log_priority);
  } else if (property_log_level >= 0) {
    return prio >= property_log_level;
  } else if (minimum_log_priority != ANDROID_LOG_DEFAULT) {
    return prio >= minimum_log_priority;
  } else {
    return prio >= default_prio;
  }
}

int __android_log_is_loggable(int prio, const char* tag, int default_prio) {
  auto len = tag ? strlen(tag) : 0;
  return __android_log_is_loggable_len(prio, tag, len, default_prio);
}

int __android_log_is_debuggable() {
  static int is_debuggable = [] {
    char value[PROP_VALUE_MAX] = {};
    return __system_property_get("ro.debuggable", value) > 0 && !strcmp(value, "1");
  }();

  return is_debuggable;
}

/*
 * For properties that are read often, but generally remain constant.
 * Since a change is rare, we will accept a trylock failure gracefully.
 * Use a separate lock from is_loggable to keep contention down b/25563384.
 */
struct cache2_char {
  pthread_mutex_t lock;
  uint32_t serial;
  const char* key_persist;
  struct cache_char cache_persist;
  const char* key_ro;
  struct cache_char cache_ro;
  unsigned char (*const evaluate)(const struct cache2_char* self);
};

static inline unsigned char do_cache2_char(struct cache2_char* self) {
  uint32_t current_serial;
  int change_detected;
  unsigned char c;

  if (pthread_mutex_trylock(&self->lock)) {
    /* We are willing to accept some race in this context */
    return self->evaluate(self);
  }

  change_detected = check_cache(&self->cache_persist.cache) || check_cache(&self->cache_ro.cache);
  current_serial = __system_property_area_serial();
  if (current_serial != self->serial) {
    change_detected = 1;
  }
  if (change_detected) {
    refresh_cache(&self->cache_persist, self->key_persist);
    refresh_cache(&self->cache_ro, self->key_ro);
    self->serial = current_serial;
  }
  c = self->evaluate(self);

  pthread_mutex_unlock(&self->lock);

  return c;
}

/*
 * Security state generally remains constant, but the DO must be able
 * to turn off logging should it become spammy after an attack is detected.
 */
static unsigned char evaluate_security(const struct cache2_char* self) {
  unsigned char c = self->cache_ro.c;

  return (c != BOOLEAN_FALSE) && c && (self->cache_persist.c == BOOLEAN_TRUE);
}

int __android_log_security() {
  static struct cache2_char security = {
      PTHREAD_MUTEX_INITIALIZER, 0,
      "persist.logd.security",   {{NULL, 0xFFFFFFFF}, BOOLEAN_FALSE},
      "ro.organization_owned",   {{NULL, 0xFFFFFFFF}, BOOLEAN_FALSE},
      evaluate_security};

  return do_cache2_char(&security);
}

#else

int __android_log_is_loggable(int prio, const char*, int) {
  int minimum_priority = __android_log_get_minimum_priority();
  if (minimum_priority == ANDROID_LOG_DEFAULT) {
    minimum_priority = ANDROID_LOG_INFO;
  }
  return prio >= minimum_priority;
}

int __android_log_is_loggable_len(int prio, const char*, size_t, int def) {
  return __android_log_is_loggable(prio, nullptr, def);
}

int __android_log_is_debuggable() {
  return 1;
}

#endif
