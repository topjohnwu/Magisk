/*
** Copyright 2012, The CyanogenMod Project
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/* reads a file, making sure it is terminated with \n \0 */
char* read_file(const char *fn)
{
    struct stat st;
    char *data = NULL;

    int fd = open(fn, O_RDONLY);
    if (fd < 0) return data;

    if (fstat(fd, &st)) goto oops;

    data = malloc(st.st_size + 2);
    if (!data) goto oops;

    if (read(fd, data, st.st_size) != st.st_size) goto oops;
    close(fd);
    data[st.st_size] = '\n';
    data[st.st_size + 1] = 0;
    return data;

oops:
    close(fd);
    if (data) free(data);
    return NULL;
}

int get_property(const char *data, char *found, const char *searchkey, const char *not_found)
{
    char *key, *value, *eol, *sol, *tmp;
    if (data == NULL) goto defval;
    int matched = 0;
    sol = strdup(data);
    while((eol = strchr(sol, '\n'))) {
        key = sol;
        *eol++ = 0;
        sol = eol;

        value = strchr(key, '=');
        if(value == 0) continue;
        *value++ = 0;

        while(isspace(*key)) key++;
        if(*key == '#') continue;
        tmp = value - 2;
        while((tmp > key) && isspace(*tmp)) *tmp-- = 0;

        while(isspace(*value)) value++;
        tmp = eol - 2;
        while((tmp > value) && isspace(*tmp)) *tmp-- = 0;

        if (strncmp(searchkey, key, strlen(searchkey)) == 0) {
            matched = 1;
            break;
        }
    }
    int len;
    if (matched) {
        len = strlen(value);
        if (len >= PROPERTY_VALUE_MAX)
            return -1;
        memcpy(found, value, len + 1);
    } else goto defval;
    return len;

defval:
    len = strlen(not_found);
    memcpy(found, not_found, len + 1);
    return len;
}

/*
 * Fast version of get_property which purpose is to check
 * whether the property with given prefix exists.
 *
 * Assume nobody is stupid enough to put a propery with prefix ro.cm.version
 * in his build.prop on a non-CM ROM and comment it out.
 */
int check_property(const char *data, const char *prefix)
{
    if (!data)
        return 0;
    return strstr(data, prefix) != NULL;
}
