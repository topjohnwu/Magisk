/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <inttypes.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

int load_canned_fs_config(const char* fn);
void canned_fs_config(const char* path, int dir, const char* target_out_path, unsigned* uid,
                      unsigned* gid, unsigned* mode, uint64_t* capabilities);

__END_DECLS
