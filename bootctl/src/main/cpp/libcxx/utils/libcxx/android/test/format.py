import os

import lit.util  # pylint: disable=import-error

from libcxx.android.executors import AdbExecutor
from libcxx.test.executor import LocalExecutor, TimeoutExecutor
import libcxx.test.format
import libcxx.android.adb as adb


class HostTestFormat(libcxx.test.format.LibcxxTestFormat):
    # pylint: disable=super-init-not-called
    def __init__(self, cxx, libcxx_src_root, libcxx_obj_root, timeout,
                 exec_env=None):
        self.cxx = cxx
        self.libcxx_src_root = libcxx_src_root
        self.libcxx_obj_root = libcxx_obj_root
        self.use_verify_for_fail = False
        self.executor = TimeoutExecutor(timeout, LocalExecutor())

        # We need to use LD_LIBRARY_PATH because the build system's rpath is
        # relative, which won't work since we're running from /tmp. We can
        # either scan `cxx_under_test`/`link_template` to determine whether
        # we're 32-bit or 64-bit, scan testconfig.mk, or just add both
        # directories and let the linker sort it out. I'm choosing the lazy
        # option.
        outdir = os.getenv('ANDROID_HOST_OUT')
        libpath = os.pathsep.join([
            os.path.join(outdir, 'lib'),
            os.path.join(outdir, 'lib64'),
        ])
        env = {'LD_LIBRARY_PATH': libpath}
        if exec_env is not None:
            env.update(exec_env)
        self.exec_env = env


class TestFormat(HostTestFormat):
    def __init__(self, cxx, libcxx_src_root, libcxx_obj_root, device_dir,
                 timeout, exec_env=None):
        HostTestFormat.__init__(
            self,
            cxx,
            libcxx_src_root,
            libcxx_obj_root,
            timeout,
            exec_env)
        self.device_dir = device_dir
        self.executor = TimeoutExecutor(timeout, AdbExecutor())

    def _working_directory(self, file_name):
        return os.path.join(self.device_dir, file_name)

    def _wd_path(self, test_name, file_name):
        return os.path.join(self._working_directory(test_name), file_name)

    def _clean(self, exec_path):
        exec_file = os.path.basename(exec_path)
        cmd = ['adb', 'shell', 'rm', '-rf', self._working_directory(exec_file)]
        lit.util.executeCommand(cmd)
        try:
            os.remove(exec_path)
        except OSError:
            pass

    def _run(self, exec_path, _, in_dir=None):
        raise NotImplementedError()
