/*
 * Copyright (C) 2019 The Android Open Source Project
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
#include <utils/Errors.h>

namespace android {

std::string statusToString(status_t s) {
#define STATUS_CASE(STATUS) \
    case STATUS:            \
        return #STATUS

    switch (s) {
        STATUS_CASE(OK);
        STATUS_CASE(UNKNOWN_ERROR);
        STATUS_CASE(NO_MEMORY);
        STATUS_CASE(INVALID_OPERATION);
        STATUS_CASE(BAD_VALUE);
        STATUS_CASE(BAD_TYPE);
        STATUS_CASE(NAME_NOT_FOUND);
        STATUS_CASE(PERMISSION_DENIED);
        STATUS_CASE(NO_INIT);
        STATUS_CASE(ALREADY_EXISTS);
        STATUS_CASE(DEAD_OBJECT);
        STATUS_CASE(FAILED_TRANSACTION);
        STATUS_CASE(BAD_INDEX);
        STATUS_CASE(NOT_ENOUGH_DATA);
        STATUS_CASE(WOULD_BLOCK);
        STATUS_CASE(TIMED_OUT);
        STATUS_CASE(UNKNOWN_TRANSACTION);
        STATUS_CASE(FDS_NOT_ALLOWED);
        STATUS_CASE(UNEXPECTED_NULL);
#undef STATUS_CASE
    }

    return std::to_string(s) + " (" + strerror(-s) + ")";
}

}  // namespace android
