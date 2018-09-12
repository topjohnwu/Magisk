/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "system_properties/context_node.h"

#include <limits.h>
#include <unistd.h>

#include <async_safe/log.h>

#include "system_properties/system_properties.h"

// pthread_mutex_lock() calls into system_properties in the case of contention.
// This creates a risk of dead lock if any system_properties functions
// use pthread locks after system_property initialization.
//
// For this reason, the below three functions use a bionic Lock and static
// allocation of memory for each filename.

bool ContextNode::Open(bool access_rw, bool* fsetxattr_failed) {
  lock_.lock();
  if (pa_) {
    lock_.unlock();
    return true;
  }

  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/%s", filename_, context_);
  if (len < 0 || len >= PROP_FILENAME_MAX) {
    lock_.unlock();
    return false;
  }

  if (access_rw) {
    pa_ = prop_area::map_prop_area_rw(filename, context_, fsetxattr_failed);
  } else {
    pa_ = prop_area::map_prop_area(filename);
  }
  lock_.unlock();
  return pa_;
}

bool ContextNode::CheckAccessAndOpen() {
  if (!pa_ && !no_access_) {
    if (!CheckAccess() || !Open(false, nullptr)) {
      no_access_ = true;
    }
  }
  return pa_;
}

void ContextNode::ResetAccess() {
  if (!CheckAccess()) {
    Unmap();
    no_access_ = true;
  } else {
    no_access_ = false;
  }
}

bool ContextNode::CheckAccess() {
  char filename[PROP_FILENAME_MAX];
  int len = async_safe_format_buffer(filename, sizeof(filename), "%s/%s", filename_, context_);
  if (len < 0 || len >= PROP_FILENAME_MAX) {
    return false;
  }

  return access(filename, R_OK) == 0;
}

void ContextNode::Unmap() {
  prop_area::unmap_prop_area(&pa_);
}
