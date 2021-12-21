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

#include <gtest/gtest.h>

#include <errno.h>
#include <stdio.h>

TEST(libc, __pstore_append) {
#ifdef __ANDROID__
#ifndef NO_PSTORE
  if (access("/dev/pmsg0", W_OK) != 0) {
    GTEST_SKIP() << "pmsg0 not found, skipping test";
  }

  FILE* fp;
  ASSERT_TRUE(NULL != (fp = fopen("/dev/pmsg0", "ae")));
  static const char message[] = "libc.__pstore_append\n";
  ASSERT_EQ((size_t)1, fwrite(message, sizeof(message), 1, fp));
  int fflushReturn = fflush(fp);
  int fflushErrno = fflushReturn ? errno : 0;
  ASSERT_EQ(0, fflushReturn);
  ASSERT_EQ(0, fflushErrno);
  int fcloseReturn = fclose(fp);
  int fcloseErrno = fcloseReturn ? errno : 0;
  ASSERT_EQ(0, fcloseReturn);
  ASSERT_EQ(0, fcloseErrno);
  if ((fcloseErrno == ENOMEM) || (fflushErrno == ENOMEM)) {
    fprintf(stderr,
            "Kernel does not have space allocated to pmsg pstore driver "
            "configured\n");
  }
  if (!fcloseReturn && !fcloseErrno && !fflushReturn && !fflushErrno) {
    fprintf(stderr,
            "Reboot, ensure string libc.__pstore_append is in "
            "/sys/fs/pstore/pmsg-ramoops-0\n");
  }
#else  /* NO_PSTORE */
  GTEST_LOG_(INFO) << "This test does nothing because of NO_PSTORE.\n";
#endif /* NO_PSTORE */
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}
