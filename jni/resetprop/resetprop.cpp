/* resetprop.cpp - Manipulate any system props
 * 
 * Copyright 2016 nkk71     <nkk71x@gmail.com>
 * Copyright 2016 topjohnwu <topjohnwu@gmail.com>
 * 
 * Info:
 * 
 * all changes are in
 *
 * bionic/libc/bionic/system_properties.cpp
 *
 * Functions that need to be patched/added in system_properties.cpp
 *
 * int __system_properties_init2()
 *     on android 7, first tear down the everything then let it initialize again:
 *         if (initialized) {
 *             //list_foreach(contexts, [](context_node* l) { l->reset_access(); });
 *             //return 0;
 *             free_and_unmap_contexts();
 *             initialized = false;
 *         }
 *
 *
 * static prop_area* map_prop_area(const char* filename, bool is_legacy)
 *     we dont want this read only so change: 'O_RDONLY' to 'O_RDWR'
 *
 * static prop_area* map_fd_ro(const int fd)
 *     we dont want this read only so change: 'PROT_READ' to 'PROT_READ | PROT_WRITE'
 * 
 *
 * Copy the code of prop_info *prop_area::find_property, and modify to delete props
 * const prop_info *prop_area::find_property_and_del(prop_bt *const trie, const char *name)
 * {
 *    ... 
 *    ...  Do not alloc a new prop_bt here, remove all code involve alloc_if_needed
 *    ...
 * 
 *     if (prop_offset != 0) {
 *         atomic_store_explicit(&current->prop, 0, memory_order_release); // Add this line to nullify the prop entry
 *         return to_prop_info(&current->prop);
 *     } else {
 *     
 *    ....
 * }
 *      
 *
 * by patching just those functions directly, all other functions should be ok
 * as is.
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "_system_properties.h"
#include <sys/system_properties.h>

#ifdef INDEP_BINARY

int resetprop_main(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    return resetprop_main(argc, argv);
}
#define PRINT_D(...)  if (verbose) printf(__VA_ARGS__)
#define PRINT_E(...)  fprintf(stderr, __VA_ARGS__)

#else
#include "magisk.h"

#define PRINT_D(...)  { LOGD(__VA_ARGS__); if (verbose) printf(__VA_ARGS__); }
#define PRINT_E(...)  { LOGE(__VA_ARGS__); fprintf(stderr, __VA_ARGS__); }

#endif

#include "resetprop.h"

static int verbose = 0;

static bool is_legal_property_name(const char* name, size_t namelen) {
    if (namelen < 1) return false;
    if (name[0] == '.') return false;
    if (name[namelen - 1] == '.') return false;

    /* Only allow alphanumeric, plus '.', '-', or '_' */
    /* Don't allow ".." to appear in a property name */
    for (size_t i = 0; i < namelen; i++) {
        if (name[i] == '.') {
            // i=0 is guaranteed to never have a dot. See above.
            if (name[i - 1] == '.') return false;
            continue;
        }
        if (name[i] == '_' || name[i] == '-') continue;
        if (name[i] >= 'a' && name[i] <= 'z') continue;
        if (name[i] >= 'A' && name[i] <= 'Z') continue;
        if (name[i] >= '0' && name[i] <= '9') continue;
        return false;
    }

    return true;
}

static int usage(char* arg0) {
    fprintf(stderr,
        "resetprop v" xstr(MAGISK_VERSION) " (by topjohnwu & nkk71) - System Props Modification Tool\n\n"
        "Usage: %s [options] [args...]\n"
        "%s <name> <value>:      Set property entry <name> with <value>\n"
        "%s --file <prop file>:  Load props from <prop file>\n"
        "%s --delete <name>:     Remove prop entry <name>\n"
        "\n"
        "Options:\n"
        "   -v          verbose output\n"
        "   -n          don't trigger events when changing props\n"
    , arg0, arg0, arg0, arg0);
    return 1;
}

static int init_resetprop() {
    if (__system_properties_init2()) {
        PRINT_E("resetprop: Initialize error\n");
        return -1;
    }
    return 0;
}

int prop_exist(const char *name) {
    if (init_resetprop()) return 0;
    return __system_property_find2(name) != NULL;
}

static void read_prop_info(void* cookie, const char *name, const char *value, uint32_t serial) {
    strcpy((char *) cookie, value);
}

// Get prop by name, return string (should free manually!)
char *getprop(const char *name) {
    if (init_resetprop()) return NULL;
    const prop_info *pi = __system_property_find2(name);
    if (pi == NULL) {
        PRINT_D("resetprop: prop [%s] does not exist\n", name);
        return NULL;
    }
    char value[PROP_VALUE_MAX];
    __system_property_read_callback2(pi, read_prop_info, value);
    PRINT_D("resetprop: getprop [%s]: [%s]\n", name, value);
    return strdup(value);
}

int setprop(const char *name, const char *value) {
    return setprop2(name, value, 1);
}

int setprop2(const char *name, const char *value, const int trigger) {
    if (init_resetprop()) return -1;
    int ret;
    
    prop_info *pi = (prop_info*) __system_property_find2(name);
    if (pi != NULL) {
        if (trigger) {
            if (!strncmp(name, "ro.", 3)) deleteprop(name);
            ret = __system_property_set2(name, value);
        } else {
            ret = __system_property_update2(pi, value, strlen(value));
        }
    } else {
        PRINT_D("resetprop: New prop [%s]\n", name);
        if (trigger) {
            ret = __system_property_set2(name, value);
        } else {
            ret = __system_property_add2(name, strlen(name), value, strlen(value));
        }
    }

    PRINT_D("resetprop: setprop [%s]: [%s] by %s\n", name, value,
        trigger ? "property_service" : "modifing prop data structure");

    if (ret)
        PRINT_E("resetprop: setprop error\n");

    return ret;
}

int deleteprop(const char *name) {
    if (init_resetprop()) return -1;
    PRINT_D("resetprop: deleteprop [%s]\n", name);
    if (__system_property_del(name)) {
        PRINT_E("resetprop: delete prop: [%s] error\n", name);
        return -1;
    }
    return 0;
}

int read_prop_file(const char* filename, const int trigger) {
    if (init_resetprop()) return -1;
    PRINT_D("resetprop: Load prop file [%s]\n", filename);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        PRINT_E("Cannot open [%s]\n", filename);
        return 1;
    }
    char *line = NULL, *pch;
    size_t len;
    ssize_t read;
    int comment = 0, i;
    while ((read = getline(&line, &len, fp)) != -1) {
        // Remove the trailing newline
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
            --read;
        }
        comment = 0;
        for (i = 0; i < read; ++i) {
            // Ignore starting spaces
            if (line[i] == ' ') continue;
            else {
                // A line starting with # is ignored
                if (line[i] == '#') comment = 1;
                break;
            }
        }
        if (comment) continue;
        pch = strchr(line, '=');
        // Ignore ivalid formats
        if ( ((pch == NULL) || (i >= (pch - line))) || (pch >= line + read - 1) ) continue;
        // Separate the string
        *pch = '\0';
        setprop2(line + i, pch + 1, trigger);
    }
    free(line);
    fclose(fp);
    return 0;
}

int resetprop_main(int argc, char *argv[]) {

    int del = 0, file = 0, trigger = 1;
    
    int exp_arg = 2;
    char *name, *value, *filename;

    if (argc < 3) {
        return usage(argv[0]);
    }

    for (int i = 1; i < argc; ++i) {
        if (!strcmp("-v", argv[i])) {
            verbose = 1;
        } else if (!strcmp("-n", argv[i])) {
            trigger = 0;
        } else if (!strcmp("--file", argv[i])) {
            file = 1;
            exp_arg = 1;
        } else if (!strcmp("--delete", argv[i])) {
            del = 1;
            exp_arg = 1;
        } else {
            if (i + exp_arg > argc) {
                return usage(argv[0]);
            }
            if (file) {
                filename = argv[i];
                break;
            } else {
                if(!is_legal_property_name(argv[i], strlen(argv[i]))) {
                    PRINT_E("Illegal property name: [%s]\n", argv[i]);
                    return 1;
                }
                name = argv[i];
                if (exp_arg > 1) {
                    if (strlen(argv[i + 1]) >= PROP_VALUE_MAX) {
                        PRINT_E("Value too long: [%s]\n", argv[i + 1]);
                        return 1;
                    }
                    value = argv[i + 1];
                }
                break;
            }
        }
    }

    if (file) {
        return read_prop_file(filename, trigger);
    } else if (del) {
        return deleteprop(name);
    } else {
        return setprop2(name, value, trigger);
    }

    return 0;
}
