/*
 * Copyright (C) 2017 The Android Open Source Project
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
#ifndef VNDKSUPPORT_LINKER_H_
#define VNDKSUPPORT_LINKER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns whether the current process is a vendor process.
 *
 * Note that this is only checking what process is running and has nothing to
 * do with what namespace the caller is loaded at.  For example, a VNDK-SP
 * library loaded by SP-HAL calling this function may still get a 'false',
 * because it is running in a system process.
 */
int android_is_in_vendor_process();

void* android_load_sphal_library(const char* name, int flag);

int android_unload_sphal_library(void* handle);

#ifdef __cplusplus
}
#endif

#endif  // VNDKSUPPORT_LINKER_H_
