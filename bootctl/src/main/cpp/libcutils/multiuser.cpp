/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <cutils/multiuser.h>
#include <private/android_filesystem_config.h>

userid_t multiuser_get_user_id(uid_t uid) {
    return uid / AID_USER_OFFSET;
}

appid_t multiuser_get_app_id(uid_t uid) {
    return uid % AID_USER_OFFSET;
}

uid_t multiuser_get_uid(userid_t user_id, appid_t app_id) {
    return (user_id * AID_USER_OFFSET) + (app_id % AID_USER_OFFSET);
}

gid_t multiuser_get_cache_gid(userid_t user_id, appid_t app_id) {
    if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
        return multiuser_get_uid(user_id, (app_id - AID_APP_START) + AID_CACHE_GID_START);
    } else {
        return -1;
    }
}

gid_t multiuser_get_ext_gid(userid_t user_id, appid_t app_id) {
    if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
        return multiuser_get_uid(user_id, (app_id - AID_APP_START) + AID_EXT_GID_START);
    } else {
        return -1;
    }
}

gid_t multiuser_get_ext_cache_gid(userid_t user_id, appid_t app_id) {
    if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
        return multiuser_get_uid(user_id, (app_id - AID_APP_START) + AID_EXT_CACHE_GID_START);
    } else {
        return -1;
    }
}

gid_t multiuser_get_shared_gid(userid_t, appid_t app_id) {
    if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
        return (app_id - AID_APP_START) + AID_SHARED_GID_START;
    } else if (app_id >= AID_ROOT && app_id <= AID_APP_START) {
        return app_id;
    } else {
        return -1;
    }
}

gid_t multiuser_get_shared_app_gid(uid_t uid) {
    return multiuser_get_shared_gid(multiuser_get_user_id(uid), multiuser_get_app_id(uid));
}
