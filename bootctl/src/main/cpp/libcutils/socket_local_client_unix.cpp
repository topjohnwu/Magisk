/*
 * Copyright (C) 2006 The Android Open Source Project
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

#include <cutils/sockets.h>

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(_WIN32)

int socket_local_client(const char *name, int namespaceId, int type)
{
    errno = ENOSYS;
    return -1;
}

#else /* !_WIN32 */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>

#include "socket_local_unix.h"

#define LISTEN_BACKLOG 4

/* Documented in header file. */
int socket_make_sockaddr_un(const char *name, int namespaceId, 
        struct sockaddr_un *p_addr, socklen_t *alen)
{
    memset (p_addr, 0, sizeof (*p_addr));
    size_t namelen;

    switch (namespaceId) {
        case ANDROID_SOCKET_NAMESPACE_ABSTRACT:
#if defined(__linux__)
            namelen  = strlen(name);

            // Test with length +1 for the *initial* '\0'.
            if ((namelen + 1) > sizeof(p_addr->sun_path)) {
                goto error;
            }

            /*
             * Note: The path in this case is *not* supposed to be
             * '\0'-terminated. ("man 7 unix" for the gory details.)
             */
            
            p_addr->sun_path[0] = 0;
            memcpy(p_addr->sun_path + 1, name, namelen);
#else
            /* this OS doesn't have the Linux abstract namespace */

            namelen = strlen(name) + strlen(FILESYSTEM_SOCKET_PREFIX);
            /* unix_path_max appears to be missing on linux */
            if (namelen > sizeof(*p_addr) 
                    - offsetof(struct sockaddr_un, sun_path) - 1) {
                goto error;
            }

            strcpy(p_addr->sun_path, FILESYSTEM_SOCKET_PREFIX);
            strcat(p_addr->sun_path, name);
#endif
        break;

        case ANDROID_SOCKET_NAMESPACE_RESERVED:
            namelen = strlen(name) + strlen(ANDROID_RESERVED_SOCKET_PREFIX);
            /* unix_path_max appears to be missing on linux */
            if (namelen > sizeof(*p_addr) 
                    - offsetof(struct sockaddr_un, sun_path) - 1) {
                goto error;
            }

            strcpy(p_addr->sun_path, ANDROID_RESERVED_SOCKET_PREFIX);
            strcat(p_addr->sun_path, name);
        break;

        case ANDROID_SOCKET_NAMESPACE_FILESYSTEM:
            namelen = strlen(name);
            /* unix_path_max appears to be missing on linux */
            if (namelen > sizeof(*p_addr) 
                    - offsetof(struct sockaddr_un, sun_path) - 1) {
                goto error;
            }

            strcpy(p_addr->sun_path, name);
        break;
        default:
            // invalid namespace id
            return -1;
    }

    p_addr->sun_family = AF_LOCAL;
    *alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    return 0;
error:
    return -1;
}

/**
 * connect to peer named "name" on fd
 * returns same fd or -1 on error.
 * fd is not closed on error. that's your job.
 * 
 * Used by AndroidSocketImpl
 */
int socket_local_client_connect(int fd, const char* name, int namespaceId, int /*type*/) {
    struct sockaddr_un addr;
    socklen_t alen;
    int err;

    err = socket_make_sockaddr_un(name, namespaceId, &addr, &alen);

    if (err < 0) {
        goto error;
    }

    if(connect(fd, (struct sockaddr *) &addr, alen) < 0) {
        goto error;
    }

    return fd;

error:
    return -1;
}

/** 
 * connect to peer named "name"
 * returns fd or -1 on error
 */
int socket_local_client(const char *name, int namespaceId, int type)
{
    int s;

    s = socket(AF_LOCAL, type, 0);
    if(s < 0) return -1;

    if ( 0 > socket_local_client_connect(s, name, namespaceId, type)) {
        close(s);
        return -1;
    }

    return s;
}

#endif /* !_WIN32 */
