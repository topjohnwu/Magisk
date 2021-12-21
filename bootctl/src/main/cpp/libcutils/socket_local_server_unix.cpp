/* libs/cutils/socket_local_server.c
**
** Copyright 2006, The Android Open Source Project
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

#include <cutils/sockets.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

#if defined(_WIN32)

int socket_local_server(const char *name, int namespaceId, int type)
{
    errno = ENOSYS;
    return -1;
}

#else /* !_WIN32 */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "socket_local_unix.h"

#define LISTEN_BACKLOG 4

/* Only the bottom bits are really the socket type; there are flags too. */
#define SOCK_TYPE_MASK 0xf

/**
 * Binds a pre-created socket(AF_LOCAL) 's' to 'name'
 * returns 's' on success, -1 on fail
 *
 * Does not call listen()
 */
int socket_local_server_bind(int s, const char *name, int namespaceId)
{
    struct sockaddr_un addr;
    socklen_t alen;
    int n;
    int err;

    err = socket_make_sockaddr_un(name, namespaceId, &addr, &alen);

    if (err < 0) {
        return -1;
    }

    /* basically: if this is a filesystem path, unlink first */
#if !defined(__linux__)
    if (1) {
#else
    if (namespaceId == ANDROID_SOCKET_NAMESPACE_RESERVED
        || namespaceId == ANDROID_SOCKET_NAMESPACE_FILESYSTEM) {
#endif
        /*ignore ENOENT*/
        unlink(addr.sun_path);
    }

    n = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

    if(bind(s, (struct sockaddr *) &addr, alen) < 0) {
        return -1;
    }

    return s;

}


/** Open a server-side UNIX domain datagram socket in the Linux non-filesystem 
 *  namespace
 *
 *  Returns fd on success, -1 on fail
 */

int socket_local_server(const char *name, int namespaceId, int type)
{
    int err;
    int s;
    
    s = socket(AF_LOCAL, type, 0);
    if (s < 0) return -1;

    err = socket_local_server_bind(s, name, namespaceId);

    if (err < 0) {
        close(s);
        return -1;
    }

    if ((type & SOCK_TYPE_MASK) == SOCK_STREAM) {
        int ret;

        ret = listen(s, LISTEN_BACKLOG);

        if (ret < 0) {
            close(s);
            return -1;
        }
    }

    return s;
}

#endif /* !_WIN32 */
