import copy
import os
import re
import shlex
import subprocess

import libcxx.compiler


class AndroidCXXCompiler(libcxx.compiler.CXXCompiler):
    def __init__(self, cxx_under_test, cxx_template, link_template):
        super(AndroidCXXCompiler, self).__init__(cxx_under_test)
        self.cxx_template = cxx_template
        self.link_template = link_template
        self.build_top = os.getenv('ANDROID_BUILD_TOP')

        # The file system tests require a handful of defines that we can't add
        # to the Android.bp since they require absolute paths.
        self.extra_cflags = []

    def copy(self):
        return copy.deepcopy(self)

    def get_triple(self):
        if 'clang' in self.path:
            return self.get_clang_triple()
        else:
            return self.get_gcc_triple()

        raise RuntimeError('Could not determine target triple.')

    def get_clang_triple(self):
        match = re.search(r'-target\s+(\S+)', self.cxx_template)
        if match:
            return match.group(1)
        return None

    def get_gcc_triple(self):
        proc = subprocess.Popen([self.path, '-v'],
                                stderr=subprocess.PIPE)
        _, stderr = proc.communicate()
        for line in stderr.split('\n'):
            print 'Checking {}'.format(line)
            match = re.search(r'^Target: (.+)$', line)
            if match:
                return match.group(1)
        return None

    def compile(self, source_files, out=None, flags=None, cwd=None):
        flags = [] if flags is None else flags
        return super(AndroidCXXCompiler, self).compile(source_files, out, flags,
                                                       self.build_top)

    def link(self, source_files, out=None, flags=None, cwd=None):
        flags = [] if flags is None else flags
        return super(AndroidCXXCompiler, self).link(source_files, out, flags,
                                                    self.build_top)

    def compileCmd(self, source_files, out=None, flags=None):
        if out is None:
            raise RuntimeError('The Android compiler requires an out path.')

        if isinstance(source_files, str):
            source_files = [source_files]
        cxx_args = self.cxx_template.replace('%OUT%', out)
        cxx_args = cxx_args.replace('%SOURCE%', ' '.join(source_files))
        return [self.path] + shlex.split(cxx_args) + self.extra_cflags

    def linkCmd(self, source_files, out=None, flags=None):
        if out is None:
            raise RuntimeError('The Android compiler requires an out path.')

        if isinstance(source_files, str):
            source_files = [source_files]
        link_args = self.link_template.replace('%OUT%', out)
        link_args = link_args.replace('%SOURCE%', ' '.join(source_files))
        return [self.path] + shlex.split(link_args)

    def _basicCmd(self, source_files, out, is_link=False, input_is_cxx=False):
        raise NotImplementedError()

    def _initTypeAndVersion(self):
        pass
