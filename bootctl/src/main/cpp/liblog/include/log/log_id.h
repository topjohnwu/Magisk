/*
 * Copyright (C) 2005-2017 The Android Open Source Project
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

#pragma once

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * log_id_t helpers
 */
log_id_t android_name_to_log_id(const char* logName);
const char* android_log_id_to_name(log_id_t log_id);

#ifdef __cplusplus
}
#endif
