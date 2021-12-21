/*
 * Copyright (C) 2020 The Android Open Source Project
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

/*
 * This file describes the project ID values we use for filesystem quota
 * tracking.  It is used on devices that don't have the sdcardfs kernel module,
 * which requires us to use filesystem project IDs for efficient quota
 * calculation.
 *
 * These values are typically set on files and directories using extended
 * attributes; see vold for examples.
 */

/* Default project ID for files on external storage. */
#define PROJECT_ID_EXT_DEFAULT 1000
/* Project ID for audio files on external storage. */
#define PROJECT_ID_EXT_MEDIA_AUDIO 1001
/* Project ID for video files on external storage. */
#define PROJECT_ID_EXT_MEDIA_VIDEO 1002
/* Project ID for image files on external storage. */
#define PROJECT_ID_EXT_MEDIA_IMAGE 1003

/* Start of project IDs for apps to mark external app data. */
#define PROJECT_ID_EXT_DATA_START 20000
/* End of project IDs for apps to mark external app data. */
#define PROJECT_ID_EXT_DATA_END 29999

/* Start of project IDs for apps to mark external cached data. */
#define PROJECT_ID_EXT_CACHE_START 30000
/* End of project IDs for apps to mark external cached data. */
#define PROJECT_ID_EXT_CACHE_END 39999

/* Start of project IDs for apps to mark external OBB data. */
#define PROJECT_ID_EXT_OBB_START 40000
/* End of project IDs for apps to mark external OBB data. */
#define PROJECT_ID_EXT_OBB_END 49999
