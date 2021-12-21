/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <cutils/str_parms.h>

#define LOG_TAG "str_params"
//#define LOG_NDEBUG 0

#define _GNU_SOURCE 1
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/hashmap.h>
#include <cutils/memory.h>
#include <log/log.h>

/* When an object is allocated but not freed in a function,
 * because its ownership is released to other object like a hashmap,
 * call RELEASE_OWNERSHIP to tell the clang analyzer and avoid
 * false warnings about potential memory leak.
 * For now, a "temporary" assignment to global variables
 * is enough to confuse the clang static analyzer.
 */
#ifdef __clang_analyzer__
static void *released_pointer;
#define RELEASE_OWNERSHIP(x) { released_pointer = x; released_pointer = 0; }
#else
#define RELEASE_OWNERSHIP(x)
#endif

struct str_parms {
    Hashmap *map;
};


static bool str_eq(void *key_a, void *key_b)
{
    return !strcmp((const char *)key_a, (const char *)key_b);
}

/* use djb hash unless we find it inadequate */
#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
static int str_hash_fn(void *str)
{
    uint32_t hash = 5381;

    for (char* p = static_cast<char*>(str); p && *p; p++)
        hash = ((hash << 5) + hash) + *p;
    return (int)hash;
}

struct str_parms *str_parms_create(void)
{
    str_parms* s = static_cast<str_parms*>(calloc(1, sizeof(str_parms)));
    if (!s) return NULL;

    s->map = hashmapCreate(5, str_hash_fn, str_eq);
    if (!s->map) {
        free(s);
        return NULL;
    }

    return s;
}

struct remove_ctxt {
    struct str_parms *str_parms;
    const char *key;
};

static bool remove_pair(void *key, void *value, void *context)
{
    remove_ctxt* ctxt = static_cast<remove_ctxt*>(context);
    bool should_continue;

    /*
     * - if key is not supplied, then we are removing all entries,
     *   so remove key and continue (i.e. return true)
     * - if key is supplied and matches, then remove it and don't
     *   continue (return false). Otherwise, return true and keep searching
     *   for key.
     *
     */
    if (!ctxt->key) {
        should_continue = true;
        goto do_remove;
    } else if (!strcmp(ctxt->key, static_cast<const char*>(key))) {
        should_continue = false;
        goto do_remove;
    }

    return true;

do_remove:
    hashmapRemove(ctxt->str_parms->map, key);
    free(key);
    free(value);
    return should_continue;
}

void str_parms_del(struct str_parms *str_parms, const char *key)
{
    struct remove_ctxt ctxt = {
        .str_parms = str_parms,
        .key = key,
    };
    hashmapForEach(str_parms->map, remove_pair, &ctxt);
}

void str_parms_destroy(struct str_parms *str_parms)
{
    struct remove_ctxt ctxt = {
        .str_parms = str_parms,
    };

    hashmapForEach(str_parms->map, remove_pair, &ctxt);
    hashmapFree(str_parms->map);
    free(str_parms);
}

struct str_parms *str_parms_create_str(const char *_string)
{
    struct str_parms *str_parms;
    char *str;
    char *kvpair;
    char *tmpstr;
    int items = 0;

    str_parms = str_parms_create();
    if (!str_parms)
        goto err_create_str_parms;

    str = strdup(_string);
    if (!str)
        goto err_strdup;

    ALOGV("%s: source string == '%s'\n", __func__, _string);

    kvpair = strtok_r(str, ";", &tmpstr);
    while (kvpair && *kvpair) {
        char *eq = strchr(kvpair, '='); /* would love strchrnul */
        char *value;
        char *key;
        void *old_val;

        if (eq == kvpair)
            goto next_pair;

        if (eq) {
            key = strndup(kvpair, eq - kvpair);
            if (*(++eq))
                value = strdup(eq);
            else
                value = strdup("");
        } else {
            key = strdup(kvpair);
            value = strdup("");
        }

        /* if we replaced a value, free it */
        old_val = hashmapPut(str_parms->map, key, value);
        RELEASE_OWNERSHIP(value);
        if (old_val) {
            free(old_val);
            free(key);
        } else {
            RELEASE_OWNERSHIP(key);
        }

        items++;
next_pair:
        kvpair = strtok_r(NULL, ";", &tmpstr);
    }

    if (!items)
        ALOGV("%s: no items found in string\n", __func__);

    free(str);

    return str_parms;

err_strdup:
    str_parms_destroy(str_parms);
err_create_str_parms:
    return NULL;
}

int str_parms_add_str(struct str_parms *str_parms, const char *key,
                      const char *value)
{
    void *tmp_key = NULL;
    void *tmp_val = NULL;
    void *old_val = NULL;

    // strdup and hashmapPut both set errno on failure.
    // Set errno to 0 so we can recognize whether anything went wrong.
    int saved_errno = errno;
    errno = 0;

    tmp_key = strdup(key);
    if (tmp_key == NULL) {
        goto clean_up;
    }

    tmp_val = strdup(value);
    if (tmp_val == NULL) {
        goto clean_up;
    }

    old_val = hashmapPut(str_parms->map, tmp_key, tmp_val);
    if (old_val == NULL) {
        // Did hashmapPut fail?
        if (errno == ENOMEM) {
            goto clean_up;
        }
        // For new keys, hashmap takes ownership of tmp_key and tmp_val.
        RELEASE_OWNERSHIP(tmp_key);
        RELEASE_OWNERSHIP(tmp_val);
        tmp_key = tmp_val = NULL;
    } else {
        // For existing keys, hashmap takes ownership of tmp_val.
        // (It also gives up ownership of old_val.)
        RELEASE_OWNERSHIP(tmp_val);
        tmp_val = NULL;
    }

clean_up:
    free(tmp_key);
    free(tmp_val);
    free(old_val);
    int result = -errno;
    errno = saved_errno;
    return result;
}

int str_parms_add_int(struct str_parms *str_parms, const char *key, int value)
{
    char val_str[12];
    int ret;

    ret = snprintf(val_str, sizeof(val_str), "%d", value);
    if (ret < 0)
        return -EINVAL;

    ret = str_parms_add_str(str_parms, key, val_str);
    return ret;
}

int str_parms_add_float(struct str_parms *str_parms, const char *key,
                        float value)
{
    char val_str[23];
    int ret;

    ret = snprintf(val_str, sizeof(val_str), "%.10f", value);
    if (ret < 0)
        return -EINVAL;

    ret = str_parms_add_str(str_parms, key, val_str);
    return ret;
}

int str_parms_has_key(struct str_parms *str_parms, const char *key) {
    return hashmapGet(str_parms->map, (void *)key) != NULL;
}

int str_parms_get_str(struct str_parms *str_parms, const char *key, char *val,
                      int len)
{
    // TODO: hashmapGet should take a const* key.
    char* value = static_cast<char*>(hashmapGet(str_parms->map, (void*)key));
    if (value)
        return strlcpy(val, value, len);

    return -ENOENT;
}

int str_parms_get_int(struct str_parms *str_parms, const char *key, int *val)
{
    char *end;

    // TODO: hashmapGet should take a const* key.
    char* value = static_cast<char*>(hashmapGet(str_parms->map, (void*)key));
    if (!value)
        return -ENOENT;

    *val = (int)strtol(value, &end, 0);
    if (*value != '\0' && *end == '\0')
        return 0;

    return -EINVAL;
}

int str_parms_get_float(struct str_parms *str_parms, const char *key,
                        float *val)
{
    float out;
    char *end;

    // TODO: hashmapGet should take a const* key.
    char* value = static_cast<char*>(hashmapGet(str_parms->map, (void*)(key)));
    if (!value)
        return -ENOENT;

    out = strtof(value, &end);
    if (*value == '\0' || *end != '\0')
        return -EINVAL;

    *val = out;
    return 0;
}

static bool combine_strings(void *key, void *value, void *context)
{
    char** old_str = static_cast<char**>(context);
    char *new_str;
    int ret;

    ret = asprintf(&new_str, "%s%s%s=%s",
                   *old_str ? *old_str : "",
                   *old_str ? ";" : "",
                   (char *)key,
                   (char *)value);
    if (*old_str)
        free(*old_str);

    if (ret >= 0) {
        *old_str = new_str;
        return true;
    }

    *old_str = NULL;
    return false;
}

char *str_parms_to_str(struct str_parms *str_parms)
{
    char *str = NULL;
    hashmapForEach(str_parms->map, combine_strings, &str);
    return (str != NULL) ? str : strdup("");
}

static bool dump_entry(void* key, void* value, void* /*context*/) {
    ALOGI("key: '%s' value: '%s'\n", (char *)key, (char *)value);
    return true;
}

void str_parms_dump(struct str_parms *str_parms)
{
    hashmapForEach(str_parms->map, dump_entry, str_parms);
}
