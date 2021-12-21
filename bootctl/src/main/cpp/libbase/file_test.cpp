/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "android-base/utf8.h"

#include <gtest/gtest.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <wchar.h>

#include <string>

#if !defined(_WIN32)
#include <pwd.h>
#else
#include <windows.h>
#endif

#include "android-base/logging.h"  // and must be after windows.h for ERROR

TEST(file, ReadFileToString_ENOENT) {
  std::string s("hello");
  errno = 0;
  ASSERT_FALSE(android::base::ReadFileToString("/proc/does-not-exist", &s));
  EXPECT_EQ(ENOENT, errno);
  EXPECT_EQ("", s);  // s was cleared.
}

TEST(file, ReadFileToString_WriteStringToFile) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;
  ASSERT_TRUE(android::base::WriteStringToFile("abc", tf.path))
    << strerror(errno);
  std::string s;
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s))
    << strerror(errno);
  EXPECT_EQ("abc", s);
}

// symlinks require elevated privileges on Windows.
#if !defined(_WIN32)
TEST(file, ReadFileToString_WriteStringToFile_symlink) {
  TemporaryFile target, link;
  ASSERT_EQ(0, unlink(link.path));
  ASSERT_EQ(0, symlink(target.path, link.path));
  ASSERT_FALSE(android::base::WriteStringToFile("foo", link.path, false));
  ASSERT_EQ(ELOOP, errno);
  ASSERT_TRUE(android::base::WriteStringToFile("foo", link.path, true));

  std::string s;
  ASSERT_FALSE(android::base::ReadFileToString(link.path, &s));
  ASSERT_EQ(ELOOP, errno);
  ASSERT_TRUE(android::base::ReadFileToString(link.path, &s, true));
  ASSERT_EQ("foo", s);
}
#endif

// WriteStringToFile2 is explicitly for setting Unix permissions, which make no
// sense on Windows.
#if !defined(_WIN32)
TEST(file, WriteStringToFile2) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;
  ASSERT_TRUE(android::base::WriteStringToFile("abc", tf.path, 0660,
                                               getuid(), getgid()))
      << strerror(errno);
  struct stat sb;
  ASSERT_EQ(0, stat(tf.path, &sb));
  ASSERT_EQ(0660U, static_cast<unsigned int>(sb.st_mode & ~S_IFMT));
  ASSERT_EQ(getuid(), sb.st_uid);
  ASSERT_EQ(getgid(), sb.st_gid);
  std::string s;
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s))
    << strerror(errno);
  EXPECT_EQ("abc", s);
}
#endif

#if defined(_WIN32)
TEST(file, NonUnicodeCharsWindows) {
  constexpr auto kMaxEnvVariableValueSize = 32767;
  std::wstring old_tmp;
  old_tmp.resize(kMaxEnvVariableValueSize);
  old_tmp.resize(GetEnvironmentVariableW(L"TMP", old_tmp.data(), old_tmp.size()));
  if (old_tmp.empty()) {
    // Can't continue with empty TMP folder.
    return;
  }

  std::wstring new_tmp = old_tmp;
  if (new_tmp.back() != L'\\') {
    new_tmp.push_back(L'\\');
  }

  {
    auto path(new_tmp + L"锦绣成都\\");
    _wmkdir(path.c_str());
    ASSERT_TRUE(SetEnvironmentVariableW(L"TMP", path.c_str()));

    TemporaryFile tf;
    ASSERT_NE(tf.fd, -1) << tf.path;
    ASSERT_TRUE(android::base::WriteStringToFd("abc", tf.fd));

    ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

    std::string s;
    ASSERT_TRUE(android::base::ReadFdToString(tf.fd, &s)) << strerror(errno);
    EXPECT_EQ("abc", s);
  }
  {
    auto path(new_tmp + L"директория с длинным именем\\");
    _wmkdir(path.c_str());
    ASSERT_TRUE(SetEnvironmentVariableW(L"TMP", path.c_str()));

    TemporaryFile tf;
    ASSERT_NE(tf.fd, -1) << tf.path;
    ASSERT_TRUE(android::base::WriteStringToFd("abc", tf.fd));

    ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

    std::string s;
    ASSERT_TRUE(android::base::ReadFdToString(tf.fd, &s)) << strerror(errno);
    EXPECT_EQ("abc", s);
  }
  {
    auto path(new_tmp + L"äüöß weiß\\");
    _wmkdir(path.c_str());
    ASSERT_TRUE(SetEnvironmentVariableW(L"TMP", path.c_str()));

    TemporaryFile tf;
    ASSERT_NE(tf.fd, -1) << tf.path;
    ASSERT_TRUE(android::base::WriteStringToFd("abc", tf.fd));

    ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

    std::string s;
    ASSERT_TRUE(android::base::ReadFdToString(tf.fd, &s)) << strerror(errno);
    EXPECT_EQ("abc", s);
  }

  SetEnvironmentVariableW(L"TMP", old_tmp.c_str());
}

TEST(file, RootDirectoryWindows) {
  constexpr auto kMaxEnvVariableValueSize = 32767;
  std::wstring old_tmp;
  bool tmp_is_empty = false;
  old_tmp.resize(kMaxEnvVariableValueSize);
  old_tmp.resize(GetEnvironmentVariableW(L"TMP", old_tmp.data(), old_tmp.size()));
  if (old_tmp.empty()) {
    tmp_is_empty = (GetLastError() == ERROR_ENVVAR_NOT_FOUND);
  }
  SetEnvironmentVariableW(L"TMP", L"C:");

  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;

  SetEnvironmentVariableW(L"TMP", tmp_is_empty ? nullptr : old_tmp.c_str());
}
#endif

TEST(file, WriteStringToFd) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;
  ASSERT_TRUE(android::base::WriteStringToFd("abc", tf.fd));

  ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

  std::string s;
  ASSERT_TRUE(android::base::ReadFdToString(tf.fd, &s)) << strerror(errno);
  EXPECT_EQ("abc", s);
}

TEST(file, WriteFully) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;
  ASSERT_TRUE(android::base::WriteFully(tf.fd, "abc", 3));

  ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

  std::string s;
  s.resize(3);
  ASSERT_TRUE(android::base::ReadFully(tf.fd, &s[0], s.size()))
    << strerror(errno);
  EXPECT_EQ("abc", s);

  ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET)) << strerror(errno);

  s.resize(1024);
  ASSERT_FALSE(android::base::ReadFully(tf.fd, &s[0], s.size()));
}

TEST(file, RemoveFileIfExists) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;
  close(tf.fd);
  tf.fd = -1;
  std::string err;
  ASSERT_TRUE(android::base::RemoveFileIfExists(tf.path, &err)) << err;
  ASSERT_TRUE(android::base::RemoveFileIfExists(tf.path));
  TemporaryDir td;
  ASSERT_FALSE(android::base::RemoveFileIfExists(td.path));
  ASSERT_FALSE(android::base::RemoveFileIfExists(td.path, &err));
  ASSERT_EQ("is not a regular file or symbolic link", err);
}

TEST(file, RemoveFileIfExists_ENOTDIR) {
  TemporaryFile tf;
  close(tf.fd);
  tf.fd = -1;
  std::string err{"xxx"};
  ASSERT_TRUE(android::base::RemoveFileIfExists(std::string{tf.path} + "/abc", &err));
  ASSERT_EQ("xxx", err);
}

#if !defined(_WIN32)
TEST(file, RemoveFileIfExists_EACCES) {
  // EACCES -- one of the directories in the path has no search permission
  // root can bypass permission restrictions, so drop root.
  if (getuid() == 0) {
    passwd* shell = getpwnam("shell");
    setgid(shell->pw_gid);
    setuid(shell->pw_uid);
  }

  TemporaryDir td;
  TemporaryFile tf(td.path);
  close(tf.fd);
  tf.fd = -1;
  std::string err{"xxx"};
  // Remove dir's search permission.
  ASSERT_TRUE(chmod(td.path, S_IRUSR | S_IWUSR) == 0);
  ASSERT_FALSE(android::base::RemoveFileIfExists(tf.path, &err));
  ASSERT_EQ("Permission denied", err);
  // Set dir's search permission again.
  ASSERT_TRUE(chmod(td.path, S_IRWXU) == 0);
  ASSERT_TRUE(android::base::RemoveFileIfExists(tf.path, &err));
}
#endif

TEST(file, Readlink) {
#if !defined(_WIN32)
  // Linux doesn't allow empty symbolic links.
  std::string min("x");
  // ext2 and ext4 both have PAGE_SIZE limits.
  // If file encryption is enabled, there's extra overhead to store the
  // size of the encrypted symlink target. There's also an off-by-one
  // in current kernels (and marlin/sailfish where we're seeing this
  // failure are still on 3.18, far from current). http://b/33306057.
  std::string max(static_cast<size_t>(4096 - 2 - 1 - 1), 'x');

  TemporaryDir td;
  std::string min_path{std::string(td.path) + "/" + "min"};
  std::string max_path{std::string(td.path) + "/" + "max"};

  ASSERT_EQ(0, symlink(min.c_str(), min_path.c_str()));
  ASSERT_EQ(0, symlink(max.c_str(), max_path.c_str()));

  std::string result;

  result = "wrong";
  ASSERT_TRUE(android::base::Readlink(min_path, &result));
  ASSERT_EQ(min, result);

  result = "wrong";
  ASSERT_TRUE(android::base::Readlink(max_path, &result));
  ASSERT_EQ(max, result);
#endif
}

TEST(file, Realpath) {
#if !defined(_WIN32)
  TemporaryDir td;
  std::string basename = android::base::Basename(td.path);
  std::string dir_name = android::base::Dirname(td.path);
  std::string base_dir_name = android::base::Basename(dir_name);

  {
    std::string path = dir_name + "/../" + base_dir_name + "/" + basename;
    std::string result;
    ASSERT_TRUE(android::base::Realpath(path, &result));
    ASSERT_EQ(td.path, result);
  }

  {
    std::string path = std::string(td.path) + "/..";
    std::string result;
    ASSERT_TRUE(android::base::Realpath(path, &result));
    ASSERT_EQ(dir_name, result);
  }

  {
    errno = 0;
    std::string path = std::string(td.path) + "/foo.noent";
    std::string result = "wrong";
    ASSERT_TRUE(!android::base::Realpath(path, &result));
    ASSERT_TRUE(result.empty());
    ASSERT_EQ(ENOENT, errno);
  }
#endif
}

TEST(file, GetExecutableDirectory) {
  std::string path = android::base::GetExecutableDirectory();
  ASSERT_NE("", path);
  ASSERT_NE(android::base::GetExecutablePath(), path);
  ASSERT_EQ('/', path[0]);
  ASSERT_NE('/', path[path.size() - 1]);
}

TEST(file, GetExecutablePath) {
  ASSERT_NE("", android::base::GetExecutablePath());
}

TEST(file, Basename) {
  EXPECT_EQ("sh", android::base::Basename("/system/bin/sh"));
  EXPECT_EQ("sh", android::base::Basename("sh"));
  EXPECT_EQ("sh", android::base::Basename("/system/bin/sh/"));
}

TEST(file, Dirname) {
  EXPECT_EQ("/system/bin", android::base::Dirname("/system/bin/sh"));
  EXPECT_EQ(".", android::base::Dirname("sh"));
  EXPECT_EQ("/system/bin", android::base::Dirname("/system/bin/sh/"));
}

TEST(file, ReadFileToString_capacity) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;

  // For a huge file, the overhead should still be small.
  std::string s;
  size_t size = 16 * 1024 * 1024;
  ASSERT_TRUE(android::base::WriteStringToFile(std::string(size, 'x'), tf.path));
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s));
  EXPECT_EQ(size, s.size());
  EXPECT_LT(s.capacity(), size + 16);

  // Even for weird badly-aligned sizes.
  size += 12345;
  ASSERT_TRUE(android::base::WriteStringToFile(std::string(size, 'x'), tf.path));
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s));
  EXPECT_EQ(size, s.size());
  EXPECT_LT(s.capacity(), size + 16);

  // We'll shrink an enormous string if you read a small file into it.
  size = 64;
  ASSERT_TRUE(android::base::WriteStringToFile(std::string(size, 'x'), tf.path));
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s));
  EXPECT_EQ(size, s.size());
  EXPECT_LT(s.capacity(), size + 16);
}

TEST(file, ReadFileToString_capacity_0) {
  TemporaryFile tf;
  ASSERT_NE(tf.fd, -1) << tf.path;

  // Because /proc reports its files as zero-length, we don't actually trust
  // any file that claims to be zero-length. Rather than add increasingly
  // complex heuristics for shrinking the passed-in string in that case, we
  // currently leave it alone.
  std::string s;
  size_t initial_capacity = s.capacity();
  ASSERT_TRUE(android::base::WriteStringToFile("", tf.path));
  ASSERT_TRUE(android::base::ReadFileToString(tf.path, &s));
  EXPECT_EQ(0U, s.size());
  EXPECT_EQ(initial_capacity, s.capacity());
}
