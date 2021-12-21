/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/qtaguid.h>

// #define LOG_NDEBUG 0

#define LOG_TAG "qtaguid"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <log/log.h>

class netdHandler {
  public:
    int (*netdTagSocket)(int, uint32_t, uid_t);
    int (*netdUntagSocket)(int);
    int (*netdSetCounterSet)(uint32_t, uid_t);
    int (*netdDeleteTagData)(uint32_t, uid_t);
};

int stubTagSocket(int, uint32_t, uid_t) {
    return -EREMOTEIO;
}

int stubUntagSocket(int) {
    return -EREMOTEIO;
}

int stubSetCounterSet(uint32_t, uid_t) {
    return -EREMOTEIO;
}

int stubDeleteTagData(uint32_t, uid_t) {
    return -EREMOTEIO;
}

netdHandler initHandler(void) {
    netdHandler handler = {stubTagSocket, stubUntagSocket, stubSetCounterSet, stubDeleteTagData};

    void* netdClientHandle = dlopen("libnetd_client.so", RTLD_NOW);
    if (!netdClientHandle) {
        ALOGE("Failed to open libnetd_client.so: %s", dlerror());
        return handler;
    }

    handler.netdTagSocket = (int (*)(int, uint32_t, uid_t))dlsym(netdClientHandle, "tagSocket");
    if (!handler.netdTagSocket) {
        ALOGE("load netdTagSocket handler failed: %s", dlerror());
    }

    handler.netdUntagSocket = (int (*)(int))dlsym(netdClientHandle, "untagSocket");
    if (!handler.netdUntagSocket) {
        ALOGE("load netdUntagSocket handler failed: %s", dlerror());
    }

    handler.netdSetCounterSet = (int (*)(uint32_t, uid_t))dlsym(netdClientHandle, "setCounterSet");
    if (!handler.netdSetCounterSet) {
        ALOGE("load netdSetCounterSet handler failed: %s", dlerror());
    }

    handler.netdDeleteTagData = (int (*)(uint32_t, uid_t))dlsym(netdClientHandle, "deleteTagData");
    if (!handler.netdDeleteTagData) {
        ALOGE("load netdDeleteTagData handler failed: %s", dlerror());
    }
    return handler;
}

// The language guarantees that this object will be initialized in a thread-safe way.
static netdHandler& getHandler() {
    static netdHandler instance = initHandler();
    return instance;
}

int qtaguid_tagSocket(int sockfd, int tag, uid_t uid) {
    // Check the socket fd passed to us is still valid before we load the netd
    // client. Pass a already closed socket fd to netd client may let netd open
    // the unix socket with the same fd number and pass it to server for
    // tagging.
    // TODO: move the check into netdTagSocket.
    int res = fcntl(sockfd, F_GETFD);
    if (res < 0) return res;

    ALOGV("Tagging socket %d with tag %u for uid %d", sockfd, tag, uid);
    return getHandler().netdTagSocket(sockfd, tag, uid);
}

int qtaguid_untagSocket(int sockfd) {
    // Similiar to tag socket. We need a check before untag to make sure untag a closed socket fail
    // as expected.
    // TODO: move the check into netdTagSocket.
    int res = fcntl(sockfd, F_GETFD);
    if (res < 0) return res;

    ALOGV("Untagging socket %d", sockfd);
    return getHandler().netdUntagSocket(sockfd);
}

int qtaguid_setCounterSet(int counterSetNum, uid_t uid) {
    ALOGV("Setting counters to set %d for uid %d", counterSetNum, uid);
    return getHandler().netdSetCounterSet(counterSetNum, uid);
}

int qtaguid_deleteTagData(int tag, uid_t uid) {
    ALOGV("Deleting tag data with tag %u for uid %d", tag, uid);
    return getHandler().netdDeleteTagData(tag, uid);
}
