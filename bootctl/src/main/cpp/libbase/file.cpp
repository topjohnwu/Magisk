/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "android-base/file.h"

#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#define O_NOFOLLOW 0
#define OS_PATH_SEPARATOR '\\'
#else
#define OS_PATH_SEPARATOR '/'
#endif

#include "android-base/logging.h"  // and must be after windows.h for ERROR
#include "android-base/macros.h"   // For TEMP_FAILURE_RETRY on Darwin.
#include "android-base/unique_fd.h"
#include "android-base/utf8.h"

namespace {

#ifdef _WIN32
static int mkstemp(char* name_template, size_t size_in_chars) {
  std::wstring path;
  CHECK(android::base::UTF8ToWide(name_template, &path))
      << "path can't be converted to wchar: " << name_template;
  if (_wmktemp_s(path.data(), path.size() + 1) != 0) {
    return -1;
  }

  // Use open() to match the close() that TemporaryFile's destructor does.
  // Use O_BINARY to match base file APIs.
  int fd = _wopen(path.c_str(), O_CREAT | O_EXCL | O_RDWR | O_BINARY, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    return -1;
  }

  std::string path_utf8;
  CHECK(android::base::WideToUTF8(path, &path_utf8)) << "path can't be converted to utf8";
  CHECK(strcpy_s(name_template, size_in_chars, path_utf8.c_str()) == 0)
      << "utf8 path can't be assigned back to name_template";

  return fd;
}

static char* mkdtemp(char* name_template, size_t size_in_chars) {
  std::wstring path;
  CHECK(android::base::UTF8ToWide(name_template, &path))
      << "path can't be converted to wchar: " << name_template;

  if (_wmktemp_s(path.data(), path.size() + 1) != 0) {
    return nullptr;
  }

  if (_wmkdir(path.c_str()) != 0) {
    return nullptr;
  }

  std::string path_utf8;
  CHECK(android::base::WideToUTF8(path, &path_utf8)) << "path can't be converted to utf8";
  CHECK(strcpy_s(name_template, size_in_chars, path_utf8.c_str()) == 0)
      << "utf8 path can't be assigned back to name_template";

  return name_template;
}
#endif

std::string GetSystemTempDir() {
#if defined(__ANDROID__)
  const auto* tmpdir = getenv("TMPDIR");
  if (tmpdir == nullptr) tmpdir = "/data/local/tmp";
  if (access(tmpdir, R_OK | W_OK | X_OK) == 0) {
    return tmpdir;
  }
  // Tests running in app context can't access /data/local/tmp,
  // so try current directory if /data/local/tmp is not accessible.
  return ".";
#elif defined(_WIN32)
  wchar_t tmp_dir_w[MAX_PATH];
  DWORD result = GetTempPathW(std::size(tmp_dir_w), tmp_dir_w);  // checks TMP env
  CHECK_NE(result, 0ul) << "GetTempPathW failed, error: " << GetLastError();
  CHECK_LT(result, std::size(tmp_dir_w)) << "path truncated to: " << result;

  // GetTempPath() returns a path with a trailing slash, but init()
  // does not expect that, so remove it.
  if (tmp_dir_w[result - 1] == L'\\') {
    tmp_dir_w[result - 1] = L'\0';
  }

  std::string tmp_dir;
  CHECK(android::base::WideToUTF8(tmp_dir_w, &tmp_dir)) << "path can't be converted to utf8";

  return tmp_dir;
#else
  const auto* tmpdir = getenv("TMPDIR");
  if (tmpdir == nullptr) tmpdir = "/tmp";
  return tmpdir;
#endif
}

}  // namespace

TemporaryFile::TemporaryFile() {
  init(GetSystemTempDir());
}

TemporaryFile::TemporaryFile(const std::string& tmp_dir) {
  init(tmp_dir);
}

TemporaryFile::~TemporaryFile() {
  if (fd != -1) {
    close(fd);
  }
  if (remove_file_) {
    unlink(path);
  }
}

int TemporaryFile::release() {
  int result = fd;
  fd = -1;
  return result;
}

void TemporaryFile::init(const std::string& tmp_dir) {
  snprintf(path, sizeof(path), "%s%cTemporaryFile-XXXXXX", tmp_dir.c_str(), OS_PATH_SEPARATOR);
#if defined(_WIN32)
  fd = mkstemp(path, sizeof(path));
#else
  fd = mkstemp(path);
#endif
}

TemporaryDir::TemporaryDir() {
  init(GetSystemTempDir());
}

TemporaryDir::~TemporaryDir() {
  if (!remove_dir_and_contents_) return;

  auto callback = [](const char* child, const struct stat*, int file_type, struct FTW*) -> int {
    switch (file_type) {
      case FTW_D:
      case FTW_DP:
      case FTW_DNR:
        if (rmdir(child) == -1) {
          PLOG(ERROR) << "rmdir " << child;
        }
        break;
      case FTW_NS:
      default:
        if (rmdir(child) != -1) break;
        // FALLTHRU (for gcc, lint, pcc, etc; and following for clang)
        FALLTHROUGH_INTENDED;
      case FTW_F:
      case FTW_SL:
      case FTW_SLN:
        if (unlink(child) == -1) {
          PLOG(ERROR) << "unlink " << child;
        }
        break;
    }
    return 0;
  };

  nftw(path, callback, 128, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
}

bool TemporaryDir::init(const std::string& tmp_dir) {
  snprintf(path, sizeof(path), "%s%cTemporaryDir-XXXXXX", tmp_dir.c_str(), OS_PATH_SEPARATOR);
#if defined(_WIN32)
  return (mkdtemp(path, sizeof(path)) != nullptr);
#else
  return (mkdtemp(path) != nullptr);
#endif
}

namespace android {
namespace base {

// Versions of standard library APIs that support UTF-8 strings.
using namespace android::base::utf8;

bool ReadFdToString(borrowed_fd fd, std::string* content) {
  content->clear();

  // Although original we had small files in mind, this code gets used for
  // very large files too, where the std::string growth heuristics might not
  // be suitable. https://code.google.com/p/android/issues/detail?id=258500.
  struct stat sb;
  if (fstat(fd.get(), &sb) != -1 && sb.st_size > 0) {
    content->reserve(sb.st_size);
  }

  char buf[BUFSIZ] __attribute__((__uninitialized__));
  ssize_t n;
  while ((n = TEMP_FAILURE_RETRY(read(fd.get(), &buf[0], sizeof(buf)))) > 0) {
    content->append(buf, n);
  }
  return (n == 0) ? true : false;
}

bool ReadFileToString(const std::string& path, std::string* content, bool follow_symlinks) {
  content->clear();

  int flags = O_RDONLY | O_CLOEXEC | O_BINARY | (follow_symlinks ? 0 : O_NOFOLLOW);
  android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), flags)));
  if (fd == -1) {
    return false;
  }
  return ReadFdToString(fd, content);
}

bool WriteStringToFd(const std::string& content, borrowed_fd fd) {
  const char* p = content.data();
  size_t left = content.size();
  while (left > 0) {
    ssize_t n = TEMP_FAILURE_RETRY(write(fd.get(), p, left));
    if (n == -1) {
      return false;
    }
    p += n;
    left -= n;
  }
  return true;
}

static bool CleanUpAfterFailedWrite(const std::string& path) {
  // Something went wrong. Let's not leave a corrupt file lying around.
  int saved_errno = errno;
  unlink(path.c_str());
  errno = saved_errno;
  return false;
}

#if !defined(_WIN32)
bool WriteStringToFile(const std::string& content, const std::string& path,
                       mode_t mode, uid_t owner, gid_t group,
                       bool follow_symlinks) {
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_BINARY |
              (follow_symlinks ? 0 : O_NOFOLLOW);
  android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), flags, mode)));
  if (fd == -1) {
    PLOG(ERROR) << "android::WriteStringToFile open failed";
    return false;
  }

  // We do an explicit fchmod here because we assume that the caller really
  // meant what they said and doesn't want the umask-influenced mode.
  if (fchmod(fd, mode) == -1) {
    PLOG(ERROR) << "android::WriteStringToFile fchmod failed";
    return CleanUpAfterFailedWrite(path);
  }
  if (fchown(fd, owner, group) == -1) {
    PLOG(ERROR) << "android::WriteStringToFile fchown failed";
    return CleanUpAfterFailedWrite(path);
  }
  if (!WriteStringToFd(content, fd)) {
    PLOG(ERROR) << "android::WriteStringToFile write failed";
    return CleanUpAfterFailedWrite(path);
  }
  return true;
}
#endif

bool WriteStringToFile(const std::string& content, const std::string& path,
                       bool follow_symlinks) {
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_BINARY |
              (follow_symlinks ? 0 : O_NOFOLLOW);
  android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), flags, 0666)));
  if (fd == -1) {
    return false;
  }
  return WriteStringToFd(content, fd) || CleanUpAfterFailedWrite(path);
}

bool ReadFully(borrowed_fd fd, void* data, size_t byte_count) {
  uint8_t* p = reinterpret_cast<uint8_t*>(data);
  size_t remaining = byte_count;
  while (remaining > 0) {
    ssize_t n = TEMP_FAILURE_RETRY(read(fd.get(), p, remaining));
    if (n <= 0) return false;
    p += n;
    remaining -= n;
  }
  return true;
}

#if defined(_WIN32)
// Windows implementation of pread. Note that this DOES move the file descriptors read position,
// but it does so atomically.
static ssize_t pread(borrowed_fd fd, void* data, size_t byte_count, off64_t offset) {
  DWORD bytes_read;
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(OVERLAPPED));
  overlapped.Offset = static_cast<DWORD>(offset);
  overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
  if (!ReadFile(reinterpret_cast<HANDLE>(_get_osfhandle(fd.get())), data,
                static_cast<DWORD>(byte_count), &bytes_read, &overlapped)) {
    // In case someone tries to read errno (since this is masquerading as a POSIX call)
    errno = EIO;
    return -1;
  }
  return static_cast<ssize_t>(bytes_read);
}
#endif

bool ReadFullyAtOffset(borrowed_fd fd, void* data, size_t byte_count, off64_t offset) {
  uint8_t* p = reinterpret_cast<uint8_t*>(data);
  while (byte_count > 0) {
    ssize_t n = TEMP_FAILURE_RETRY(pread(fd.get(), p, byte_count, offset));
    if (n <= 0) return false;
    p += n;
    byte_count -= n;
    offset += n;
  }
  return true;
}

bool WriteFully(borrowed_fd fd, const void* data, size_t byte_count) {
  const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
  size_t remaining = byte_count;
  while (remaining > 0) {
    ssize_t n = TEMP_FAILURE_RETRY(write(fd.get(), p, remaining));
    if (n == -1) return false;
    p += n;
    remaining -= n;
  }
  return true;
}

bool RemoveFileIfExists(const std::string& path, std::string* err) {
  struct stat st;
#if defined(_WIN32)
  // TODO: Windows version can't handle symbolic links correctly.
  int result = stat(path.c_str(), &st);
  bool file_type_removable = (result == 0 && S_ISREG(st.st_mode));
#else
  int result = lstat(path.c_str(), &st);
  bool file_type_removable = (result == 0 && (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)));
#endif
  if (result == -1) {
    if (errno == ENOENT || errno == ENOTDIR) return true;
    if (err != nullptr) *err = strerror(errno);
    return false;
  }

  if (result == 0) {
    if (!file_type_removable) {
      if (err != nullptr) {
        *err = "is not a regular file or symbolic link";
      }
      return false;
    }
    if (unlink(path.c_str()) == -1) {
      if (err != nullptr) {
        *err = strerror(errno);
      }
      return false;
    }
  }
  return true;
}

#if !defined(_WIN32)
bool Readlink(const std::string& path, std::string* result) {
  result->clear();

  // Most Linux file systems (ext2 and ext4, say) limit symbolic links to
  // 4095 bytes. Since we'll copy out into the string anyway, it doesn't
  // waste memory to just start there. We add 1 so that we can recognize
  // whether it actually fit (rather than being truncated to 4095).
  std::vector<char> buf(4095 + 1);
  while (true) {
    ssize_t size = readlink(path.c_str(), &buf[0], buf.size());
    // Unrecoverable error?
    if (size == -1) return false;
    // It fit! (If size == buf.size(), it may have been truncated.)
    if (static_cast<size_t>(size) < buf.size()) {
      result->assign(&buf[0], size);
      return true;
    }
    // Double our buffer and try again.
    buf.resize(buf.size() * 2);
  }
}
#endif

#if !defined(_WIN32)
bool Realpath(const std::string& path, std::string* result) {
  result->clear();

  // realpath may exit with EINTR. Retry if so.
  char* realpath_buf = nullptr;
  do {
    realpath_buf = realpath(path.c_str(), nullptr);
  } while (realpath_buf == nullptr && errno == EINTR);

  if (realpath_buf == nullptr) {
    return false;
  }
  result->assign(realpath_buf);
  free(realpath_buf);
  return true;
}
#endif

std::string GetExecutablePath() {
#if defined(__linux__)
  std::string path;
  android::base::Readlink("/proc/self/exe", &path);
  return path;
#elif defined(__APPLE__)
  char path[PATH_MAX + 1];
  uint32_t path_len = sizeof(path);
  int rc = _NSGetExecutablePath(path, &path_len);
  if (rc < 0) {
    std::unique_ptr<char> path_buf(new char[path_len]);
    _NSGetExecutablePath(path_buf.get(), &path_len);
    return path_buf.get();
  }
  return path;
#elif defined(_WIN32)
  char path[PATH_MAX + 1];
  DWORD result = GetModuleFileName(NULL, path, sizeof(path) - 1);
  if (result == 0 || result == sizeof(path) - 1) return "";
  path[PATH_MAX - 1] = 0;
  return path;
#else
#error unknown OS
#endif
}

std::string GetExecutableDirectory() {
  return Dirname(GetExecutablePath());
}

std::string Basename(const std::string& path) {
  // Copy path because basename may modify the string passed in.
  std::string result(path);

#if !defined(__BIONIC__)
  // Use lock because basename() may write to a process global and return a
  // pointer to that. Note that this locking strategy only works if all other
  // callers to basename in the process also grab this same lock, but its
  // better than nothing.  Bionic's basename returns a thread-local buffer.
  static std::mutex& basename_lock = *new std::mutex();
  std::lock_guard<std::mutex> lock(basename_lock);
#endif

  // Note that if std::string uses copy-on-write strings, &str[0] will cause
  // the copy to be made, so there is no chance of us accidentally writing to
  // the storage for 'path'.
  char* name = basename(&result[0]);

  // In case basename returned a pointer to a process global, copy that string
  // before leaving the lock.
  result.assign(name);

  return result;
}

std::string Dirname(const std::string& path) {
  // Copy path because dirname may modify the string passed in.
  std::string result(path);

#if !defined(__BIONIC__)
  // Use lock because dirname() may write to a process global and return a
  // pointer to that. Note that this locking strategy only works if all other
  // callers to dirname in the process also grab this same lock, but its
  // better than nothing.  Bionic's dirname returns a thread-local buffer.
  static std::mutex& dirname_lock = *new std::mutex();
  std::lock_guard<std::mutex> lock(dirname_lock);
#endif

  // Note that if std::string uses copy-on-write strings, &str[0] will cause
  // the copy to be made, so there is no chance of us accidentally writing to
  // the storage for 'path'.
  char* parent = dirname(&result[0]);

  // In case dirname returned a pointer to a process global, copy that string
  // before leaving the lock.
  result.assign(parent);

  return result;
}

}  // namespace base
}  // namespace android
