/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <sys/cdefs.h>
#include <unistd.h>

#include "log/log_read.h"

__BEGIN_DECLS

int LogdRead(struct logger_list* logger_list, struct log_msg* log_msg);
void LogdClose(struct logger_list* logger_list);

ssize_t SendLogdControlMessage(char* buf, size_t buf_size);

__END_DECLS
