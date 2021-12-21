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

#ifndef __SOCKET_LOCAL_H
#define __SOCKET_LOCAL_H

#define FILESYSTEM_SOCKET_PREFIX "/tmp/" 
#define ANDROID_RESERVED_SOCKET_PREFIX "/dev/socket/"

/*
 * Set up a given sockaddr_un, to have it refer to the given
 * name in the given namespace. The namespace must be one
 * of <code>ANDROID_SOCKET_NAMESPACE_ABSTRACT</code>,
 * <code>ANDROID_SOCKET_NAMESPACE_RESERVED</code>, or
 * <code>ANDROID_SOCKET_NAMESPACE_FILESYSTEM</code>. Upon success,
 * the pointed at sockaddr_un is filled in and the pointed at
 * socklen_t is set to indicate the final length. This function
 * will fail if the namespace is invalid (not one of the indicated
 * constants) or if the name is too long.
 * 
 * @return 0 on success or -1 on failure
 */ 
int socket_make_sockaddr_un(const char *name, int namespaceId, 
        struct sockaddr_un *p_addr, socklen_t *alen);

#endif
