/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <cutils/ashmem.h>

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */
#define LOG_TAG "ashmem"

#include <errno.h>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <linux/memfd.h>
#include <log/log.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>

/* Will be added to UAPI once upstream change is merged */
#define F_SEAL_FUTURE_WRITE 0x0010

/*
 * The minimum vendor API level at and after which it is safe to use memfd.
 * This is to facilitate deprecation of ashmem.
 */
#define MIN_MEMFD_VENDOR_API_LEVEL 29
#define MIN_MEMFD_VENDOR_API_LEVEL_CHAR 'Q'

/* ashmem identity */
static dev_t __ashmem_rdev;
/*
 * If we trigger a signal handler in the middle of locked activity and the
 * signal handler calls ashmem, we could get into a deadlock state.
 */
static pthread_mutex_t __ashmem_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * has_memfd_support() determines if the device can use memfd. memfd support
 * has been there for long time, but certain things in it may be missing.  We
 * check for needed support in it. Also we check if the VNDK version of
 * libcutils being used is new enough, if its not, then we cannot use memfd
 * since the older copies may be using ashmem so we just use ashmem. Once all
 * Android devices that are getting updates are new enough (ex, they were
 * originally shipped with Android release > P), then we can just use memfd and
 * delete all ashmem code from libcutils (while preserving the interface).
 *
 * NOTE:
 * The sys.use_memfd property is set by default to false in Android
 * to temporarily disable memfd, till vendor and apps are ready for it.
 * The main issue: either apps or vendor processes can directly make ashmem
 * IOCTLs on FDs they receive by assuming they are ashmem, without going
 * through libcutils. Such fds could have very well be originally created with
 * libcutils hence they could be memfd. Thus the IOCTLs will break.
 *
 * Set default value of sys.use_memfd property to true once the issue is
 * resolved, so that the code can then self-detect if kernel support is present
 * on the device. The property can also set to true from adb shell, for
 * debugging.
 */

static bool debug_log = false;            /* set to true for verbose logging and other debug  */
static bool pin_deprecation_warn = true; /* Log the pin deprecation warning only once */

/* Determine if vendor processes would be ok with memfd in the system:
 *
 * If VNDK is using older libcutils, don't use memfd. This is so that the
 * same shared memory mechanism is used across binder transactions between
 * vendor partition processes and system partition processes.
 */
static bool check_vendor_memfd_allowed() {
    std::string vndk_version = android::base::GetProperty("ro.vndk.version", "");

    if (vndk_version == "") {
        ALOGE("memfd: ro.vndk.version not defined or invalid (%s), this is mandated since P.\n",
              vndk_version.c_str());
        return false;
    }

    /* No issues if vendor is targetting current Dessert */
    if (vndk_version == "current") {
        return false;
    }

    /* Check if VNDK version is a number and act on it */
    char* p;
    long int vers = strtol(vndk_version.c_str(), &p, 10);
    if (*p == 0) {
        if (vers < MIN_MEMFD_VENDOR_API_LEVEL) {
            ALOGI("memfd: device VNDK version (%s) is < Q so using ashmem.\n",
                  vndk_version.c_str());
            return false;
        }

        return true;
    }

    // Non-numeric should be a single ASCII character. Characters after the
    // first are ignored.
    if (tolower(vndk_version[0]) < 'a' || tolower(vndk_version[0]) > 'z') {
        ALOGE("memfd: ro.vndk.version not defined or invalid (%s), this is mandated since P.\n",
              vndk_version.c_str());
        return false;
    }

    if (tolower(vndk_version[0]) < tolower(MIN_MEMFD_VENDOR_API_LEVEL_CHAR)) {
        ALOGI("memfd: device is using VNDK version (%s) which is less than Q. Use ashmem only.\n",
              vndk_version.c_str());
        return false;
    }

    return true;
}


/* Determine if memfd can be supported. This is just one-time hardwork
 * which will be cached by the caller.
 */
static bool __has_memfd_support() {
    if (check_vendor_memfd_allowed() == false) {
        return false;
    }

    /* Used to turn on/off the detection at runtime, in the future this
     * property will be removed once we switch everything over to ashmem.
     * Currently it is used only for debugging to switch the system over.
     */
    if (!android::base::GetBoolProperty("sys.use_memfd", false)) {
        if (debug_log) {
            ALOGD("sys.use_memfd=false so memfd disabled\n");
        }
        return false;
    }

    // Check if kernel support exists, otherwise fall back to ashmem.
    // This code needs to build on old API levels, so we can't use the libc
    // wrapper.
    android::base::unique_fd fd(
            syscall(__NR_memfd_create, "test_android_memfd", MFD_CLOEXEC | MFD_ALLOW_SEALING));
    if (fd == -1) {
        ALOGE("memfd_create failed: %s, no memfd support.\n", strerror(errno));
        return false;
    }

    if (fcntl(fd, F_ADD_SEALS, F_SEAL_FUTURE_WRITE) == -1) {
        ALOGE("fcntl(F_ADD_SEALS) failed: %s, no memfd support.\n", strerror(errno));
        return false;
    }

    if (debug_log) {
        ALOGD("memfd: device has memfd support, using it\n");
    }
    return true;
}

static bool has_memfd_support() {
    /* memfd_supported is the initial global per-process state of what is known
     * about memfd.
     */
    static bool memfd_supported = __has_memfd_support();

    return memfd_supported;
}

static std::string get_ashmem_device_path() {
    static const std::string boot_id_path = "/proc/sys/kernel/random/boot_id";
    std::string boot_id;
    if (!android::base::ReadFileToString(boot_id_path, &boot_id)) {
        ALOGE("Failed to read %s: %s.\n", boot_id_path.c_str(), strerror(errno));
        return "";
    };
    boot_id = android::base::Trim(boot_id);

    return "/dev/ashmem" + boot_id;
}

/* logistics of getting file descriptor for ashmem */
static int __ashmem_open_locked()
{
    static const std::string ashmem_device_path = get_ashmem_device_path();

    if (ashmem_device_path.empty()) {
        return -1;
    }

    int fd = TEMP_FAILURE_RETRY(open(ashmem_device_path.c_str(), O_RDWR | O_CLOEXEC));

    // fallback for APEX w/ use_vendor on Q, which would have still used /dev/ashmem
    if (fd < 0) {
        int saved_errno = errno;
        fd = TEMP_FAILURE_RETRY(open("/dev/ashmem", O_RDWR | O_CLOEXEC));
        if (fd < 0) {
            /* Q launching devices and newer must not reach here since they should have been
             * able to open ashmem_device_path */
            ALOGE("Unable to open ashmem device %s (error = %s) and /dev/ashmem(error = %s)",
                  ashmem_device_path.c_str(), strerror(saved_errno), strerror(errno));
            return fd;
        }
    }
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(fstat(fd, &st));
    if (ret < 0) {
        int save_errno = errno;
        close(fd);
        errno = save_errno;
        return ret;
    }
    if (!S_ISCHR(st.st_mode) || !st.st_rdev) {
        close(fd);
        errno = ENOTTY;
        return -1;
    }

    __ashmem_rdev = st.st_rdev;
    return fd;
}

static int __ashmem_open()
{
    int fd;

    pthread_mutex_lock(&__ashmem_lock);
    fd = __ashmem_open_locked();
    pthread_mutex_unlock(&__ashmem_lock);

    return fd;
}

/* Make sure file descriptor references ashmem, negative number means false */
static int __ashmem_is_ashmem(int fd, int fatal)
{
    dev_t rdev;
    struct stat st;

    if (fstat(fd, &st) < 0) {
        return -1;
    }

    rdev = 0; /* Too much complexity to sniff __ashmem_rdev */
    if (S_ISCHR(st.st_mode) && st.st_rdev) {
        pthread_mutex_lock(&__ashmem_lock);
        rdev = __ashmem_rdev;
        if (rdev) {
            pthread_mutex_unlock(&__ashmem_lock);
        } else {
            int fd = __ashmem_open_locked();
            if (fd < 0) {
                pthread_mutex_unlock(&__ashmem_lock);
                return -1;
            }
            rdev = __ashmem_rdev;
            pthread_mutex_unlock(&__ashmem_lock);

            close(fd);
        }

        if (st.st_rdev == rdev) {
            return 0;
        }
    }

    if (fatal) {
        if (rdev) {
            LOG_ALWAYS_FATAL("illegal fd=%d mode=0%o rdev=%d:%d expected 0%o %d:%d",
              fd, st.st_mode, major(st.st_rdev), minor(st.st_rdev),
              S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRGRP,
              major(rdev), minor(rdev));
        } else {
            LOG_ALWAYS_FATAL("illegal fd=%d mode=0%o rdev=%d:%d expected 0%o",
              fd, st.st_mode, major(st.st_rdev), minor(st.st_rdev),
              S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRGRP);
        }
        /* NOTREACHED */
    }

    errno = ENOTTY;
    return -1;
}

static int __ashmem_check_failure(int fd, int result)
{
    if (result == -1 && errno == ENOTTY) __ashmem_is_ashmem(fd, 1);
    return result;
}

static bool memfd_is_ashmem(int fd) {
    static bool fd_check_error_once = false;

    if (__ashmem_is_ashmem(fd, 0) == 0) {
        if (!fd_check_error_once) {
            ALOGE("memfd: memfd expected but ashmem fd used - please use libcutils.\n");
            fd_check_error_once = true;
        }

        return true;
    }

    return false;
}

int ashmem_valid(int fd)
{
    if (has_memfd_support() && !memfd_is_ashmem(fd)) {
        return 1;
    }

    return __ashmem_is_ashmem(fd, 0) >= 0;
}

static int memfd_create_region(const char* name, size_t size) {
    // This code needs to build on old API levels, so we can't use the libc
    // wrapper.
    android::base::unique_fd fd(syscall(__NR_memfd_create, name, MFD_CLOEXEC | MFD_ALLOW_SEALING));

    if (fd == -1) {
        ALOGE("memfd_create(%s, %zd) failed: %s\n", name, size, strerror(errno));
        return -1;
    }

    if (ftruncate(fd, size) == -1) {
        ALOGE("ftruncate(%s, %zd) failed for memfd creation: %s\n", name, size, strerror(errno));
        return -1;
    }

    if (debug_log) {
        ALOGE("memfd_create(%s, %zd) success. fd=%d\n", name, size, fd.get());
    }
    return fd.release();
}

/*
 * ashmem_create_region - creates a new ashmem region and returns the file
 * descriptor, or <0 on error
 *
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 */
int ashmem_create_region(const char *name, size_t size)
{
    int ret, save_errno;

    if (has_memfd_support()) {
        return memfd_create_region(name ? name : "none", size);
    }

    int fd = __ashmem_open();
    if (fd < 0) {
        return fd;
    }

    if (name) {
        char buf[ASHMEM_NAME_LEN] = {0};

        strlcpy(buf, name, sizeof(buf));
        ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_NAME, buf));
        if (ret < 0) {
            goto error;
        }
    }

    ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_SIZE, size));
    if (ret < 0) {
        goto error;
    }

    return fd;

error:
    save_errno = errno;
    close(fd);
    errno = save_errno;
    return ret;
}

static int memfd_set_prot_region(int fd, int prot) {
    /* Only proceed if an fd needs to be write-protected */
    if (prot & PROT_WRITE) {
        return 0;
    }

    if (fcntl(fd, F_ADD_SEALS, F_SEAL_FUTURE_WRITE) == -1) {
        ALOGE("memfd_set_prot_region(%d, %d): F_SEAL_FUTURE_WRITE seal failed: %s\n", fd, prot,
              strerror(errno));
        return -1;
    }

    return 0;
}

int ashmem_set_prot_region(int fd, int prot)
{
    if (has_memfd_support() && !memfd_is_ashmem(fd)) {
        return memfd_set_prot_region(fd, prot);
    }

    return __ashmem_check_failure(fd, TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_PROT_MASK, prot)));
}

int ashmem_pin_region(int fd, size_t offset, size_t len)
{
    if (!pin_deprecation_warn || debug_log) {
        ALOGE("Pinning is deprecated since Android Q. Please use trim or other methods.\n");
        pin_deprecation_warn = true;
    }

    if (has_memfd_support() && !memfd_is_ashmem(fd)) {
        return 0;
    }

    // TODO: should LP64 reject too-large offset/len?
    ashmem_pin pin = { static_cast<uint32_t>(offset), static_cast<uint32_t>(len) };
    return __ashmem_check_failure(fd, TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_PIN, &pin)));
}

int ashmem_unpin_region(int fd, size_t offset, size_t len)
{
    if (!pin_deprecation_warn || debug_log) {
        ALOGE("Pinning is deprecated since Android Q. Please use trim or other methods.\n");
        pin_deprecation_warn = true;
    }

    if (has_memfd_support() && !memfd_is_ashmem(fd)) {
        return 0;
    }

    // TODO: should LP64 reject too-large offset/len?
    ashmem_pin pin = { static_cast<uint32_t>(offset), static_cast<uint32_t>(len) };
    return __ashmem_check_failure(fd, TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_UNPIN, &pin)));
}

int ashmem_get_size_region(int fd)
{
    if (has_memfd_support() && !memfd_is_ashmem(fd)) {
        struct stat sb;

        if (fstat(fd, &sb) == -1) {
            ALOGE("ashmem_get_size_region(%d): fstat failed: %s\n", fd, strerror(errno));
            return -1;
        }

        if (debug_log) {
            ALOGD("ashmem_get_size_region(%d): %d\n", fd, static_cast<int>(sb.st_size));
        }

        return sb.st_size;
    }

    return __ashmem_check_failure(fd, TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_GET_SIZE, NULL)));
}
