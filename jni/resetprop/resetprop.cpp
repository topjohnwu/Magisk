/*
 *
 * resetprop.cpp
 * 
 * Copyright 2016 nkk71     <nkk71x@gmail.com>
 * Copyright 2016 topjohnwu <topjohnwu#gmail.com>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "_system_properties.h"
#include <sys/system_properties.h>

/* Info:
 * 
 * all changes are in
 *
 * bionic/libc/bionic/system_properties.cpp
 *
 * Functions that need to be patched/added in system_properties.cpp
 *
 * int __system_properties_init()
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

int verbose = 0, del = 0, file = 0, trigger = 1;

static bool is_legal_property_name(const char* name, size_t namelen)
{
    size_t i;
    if (namelen >= PROP_NAME_MAX) return false;
    if (namelen < 1) return false;
    if (name[0] == '.') return false;
    if (name[namelen - 1] == '.') return false;

    /* Only allow alphanumeric, plus '.', '-', or '_' */
    /* Don't allow ".." to appear in a property name */
    for (i = 0; i < namelen; i++) {
        if (name[i] == '.') {
            // i=0 is guaranteed to never have a dot. See above.
            if (name[i-1] == '.') return false;
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

int x_property_set(const char *name, const char *value)
{
    int ret;
    char value_read[PROP_VALUE_MAX];

    size_t namelen = strlen(name);
    size_t valuelen = strlen(value);

    if (trigger) {
        printf("Set with property_service: '%s'='%s'\n", name, value);
    } else {
        printf("Modify data structure: '%s'='%s'\n", name, value);
    }

    __system_property_get(name, value_read);

    if(strlen(value_read)) {
        printf("Existing property: '%s'='%s'\n", name, value_read);
        if (trigger) {
            if (!strncmp(name, "ro.", 3)) __system_property_del(name); // Only delete ro props
            ret = __system_property_set(name, value);
        } else {
            ret = __system_property_update((prop_info*) __system_property_find(name), value, valuelen);
        }
    } else {
        if (trigger) {
            ret = __system_property_set(name, value);
        } else {
            ret = __system_property_add(name, namelen, value, valuelen);
        }
    }

    if (ret != 0) {
        fprintf(stderr, "Failed to set '%s'='%s'\n", name, value);
        return ret;
    }

    __system_property_get(name, value_read);
    printf("Recheck property: '%s'='%s'\n", name, value_read);

    return 0;
}

int read_prop_file(const char* filename) {
    printf("   Attempting to read props from \'%s\'\n", filename);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open \'%s\'\n", filename);
        return 1;
    }
    char *line = NULL, *pch, name[PROP_NAME_MAX], value[PROP_VALUE_MAX];
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
        x_property_set(line + i, pch + 1);
    }
    free(line);
    fclose(fp);
    return 0;
}

int usage(char* name) {
    fprintf(stderr, "usage: %s [-v] [-n] [--file propfile] [--delete name] [ name value ] \n", name);
    fprintf(stderr, "   -v :\n");
    fprintf(stderr, "      verbose output (Default: Disabled)\n");
    fprintf(stderr, "   -n :\n");
    fprintf(stderr, "      no event triggers when changing props (Default: Will trigger events)\n");
    fprintf(stderr, "   --file propfile :\n");
    fprintf(stderr, "      Read props from prop files (e.g. build.prop)\n");
    fprintf(stderr, "   --delete name :\n");
    fprintf(stderr, "      Remove a prop entry\n\n");
    return 1;
}

int main(int argc, char *argv[])
{
    
    int exp_arg = 2, stdout_bak, null;
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
                    fprintf(stderr, "Illegal property name \'%s\'\n", argv[i]);
                    return 1;
                }
                name = argv[i];
                if (exp_arg > 1) {
                    if (strlen(argv[i + 1]) >= PROP_VALUE_MAX) {
                        fprintf(stderr, "Value too long \'%s\'\n", argv[i + 1]);
                        return 1;
                    }
                    value = argv[i + 1];
                }
                break;
            }
        }
    }

    if (!verbose) {
        fflush(stdout);
        stdout_bak = dup(1);
        null = open("/dev/null", O_WRONLY);
        dup2(null, 1);
    }

    printf("resetprop by nkk71 & topjohnwu\n");

    printf("Initializing...\n");
    if (__system_properties_init()) {
        fprintf(stderr, "Error during init\n");
        return 1;
    }

    if (file) {
        if (read_prop_file(filename)) return 1;
    } else if (del) {
        printf("Attempting to delete '%s'\n", name);
        if (__system_property_del(name)) return 1;
    } else {
        if(x_property_set(name, value)) return 1;
    }
    printf("Done!\n\n");
    return 0;

}
