#!/usr/bin/env python3
#
# Copyright (C) 2020 The Android Open Source Project
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

import subprocess
import unittest

def run_cmd(cmd, ignore_error=False):
    print("Running command:", cmd)
    p = subprocess.Popen(cmd, shell=True)
    p.communicate()
    if not ignore_error and p.returncode:
        raise subprocess.CalledProcessError(p.returncode, cmd)
    return p.returncode

class TestFmq(unittest.TestCase):
    pass

def make_test(client, server):
    def test(self):
        try:
            run_cmd("adb shell killall %s >/dev/null 2>&1" % client, ignore_error=True)
            run_cmd("adb shell killall %s >/dev/null 2>&1" % server, ignore_error=True)
            run_cmd("adb shell \"( %s ) </dev/null >/dev/null 2>&1 &\"" % server)
            run_cmd("adb shell %s" % client)
        finally:
            run_cmd("adb shell killall %s >/dev/null 2>&1" % client, ignore_error=True)
            run_cmd("adb shell killall %s >/dev/null 2>&1" % server, ignore_error=True)
    return test

def has_bitness(bitness):
    return 0 == run_cmd("echo '[[ \"$(getprop ro.product.cpu.abilist%s)\" != \"\" ]]' | adb shell sh" % bitness, ignore_error=True)

if __name__ == '__main__':
    clients = []
    servers = []

    if has_bitness(32):
        clients += ["/data/nativetest/fmq_test_client/fmq_test_client"]
        servers += ["/data/nativetest/android.hardware.tests.msgq@1.0-service-test/android.hardware.tests.msgq@1.0-service-test"]

    if has_bitness(64):
        clients += ["/data/nativetest64/fmq_test_client/fmq_test_client"]
        servers += ["/data/nativetest64/android.hardware.tests.msgq@1.0-service-test/android.hardware.tests.msgq@1.0-service-test"]

    assert len(clients) > 0
    assert len(servers) > 0

    def short_name(binary):
        if "64" in binary:
            return "64"
        return "32"

    for client in clients:
        for server in servers:
            test_name = 'test_%s_to_%s' % (short_name(client), short_name(server))
            test = make_test(client, server)
            setattr(TestFmq, test_name, test)

    suite = unittest.TestLoader().loadTestsFromTestCase(TestFmq)
    unittest.TextTestRunner(verbosity=2).run(suite)
