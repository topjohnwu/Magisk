#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    msgq_benchmark_client.cpp

LOCAL_CFLAGS := -Wall -Werror

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libutils \
    libhidlbase

LOCAL_REQUIRED_MODULES := android.hardware.tests.msgq@1.0-impl

ifneq ($(TARGET_2ND_ARCH),)
LOCAL_REQUIRED_MODULES += android.hardware.tests.msgq@1.0-impl:32
endif

LOCAL_SHARED_LIBRARIES += android.hardware.tests.msgq@1.0 libfmq
LOCAL_MODULE := mq_benchmark_client
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0
LOCAL_LICENSE_CONDITIONS := notice
include $(BUILD_NATIVE_TEST)
