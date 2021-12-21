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

#pragma once

#include <stdint.h>

__BEGIN_DECLS

// For host builds, __INTRODUCED_IN is not defined.
#ifndef __INTRODUCED_IN
#define __INTRODUCED_IN(x)
#endif

struct ACgroupController;
typedef struct ACgroupController ACgroupController;

// ACgroupFile

/**
 * Returns file version. See android::cgrouprc::format::CgroupFile for a list of valid versions
 * for the file.
 * If ACgroupFile_init() isn't called, initialization will be done first.
 * If initialization failed, return 0.
 */
__attribute__((warn_unused_result)) uint32_t ACgroupFile_getVersion() __INTRODUCED_IN(29);

/**
 * Returns the number of controllers.
 * If ACgroupFile_init() isn't called, initialization will be done first.
 * If initialization failed, return 0.
 */
__attribute__((warn_unused_result)) uint32_t ACgroupFile_getControllerCount() __INTRODUCED_IN(29);

/**
 * Returns the controller at the given index.
 * Returnss nullptr if the given index exceeds getControllerCount().
 * If ACgroupFile_init() isn't called, initialization will be done first.
 * If initialization failed, return 0.
 */
__attribute__((warn_unused_result)) const ACgroupController* ACgroupFile_getController(
        uint32_t index) __INTRODUCED_IN(29);

// ACgroupController

/**
 * Returns the version of the given controller.
 * If the given controller is null, return 0.
 */
__attribute__((warn_unused_result)) uint32_t ACgroupController_getVersion(const ACgroupController*)
        __INTRODUCED_IN(29);

/**
 * Flag bitmask used in ACgroupController_getFlags
 */
#define CGROUPRC_CONTROLLER_FLAG_MOUNTED 0x1
#define CGROUPRC_CONTROLLER_FLAG_NEEDS_ACTIVATION 0x2
#define CGROUPRC_CONTROLLER_FLAG_OPTIONAL 0x4

/**
 * Returns the flags bitmask of the given controller.
 * If the given controller is null, return 0.
 */
__attribute__((warn_unused_result, weak)) uint32_t ACgroupController_getFlags(
        const ACgroupController*) __INTRODUCED_IN(30);

/**
 * Returns the name of the given controller.
 * If the given controller is null, return nullptr.
 */
__attribute__((warn_unused_result)) const char* ACgroupController_getName(const ACgroupController*)
        __INTRODUCED_IN(29);

/**
 * Returns the path of the given controller.
 * If the given controller is null, return nullptr.
 */
__attribute__((warn_unused_result)) const char* ACgroupController_getPath(const ACgroupController*)
        __INTRODUCED_IN(29);

__END_DECLS
