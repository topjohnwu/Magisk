#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

"""not.py is a utility for inverting the return code of commands.
It acts similar to llvm/utils/not.
ex: python /path/to/not.py ' echo hello
    echo $? // (prints 1)
"""

import distutils.spawn
import subprocess
import sys


def main():
    argv = list(sys.argv)
    del argv[0]
    if len(argv) > 0 and argv[0] == '--crash':
        del argv[0]
        expectCrash = True
    else:
        expectCrash = False
    if len(argv) == 0:
        return 1
    prog = distutils.spawn.find_executable(argv[0])
    if prog is None:
        sys.stderr.write('Failed to find program %s' % argv[0])
        return 1
    rc = subprocess.call(argv)
    if rc < 0:
        return 0 if expectCrash else 1
    if expectCrash:
        return 1
    return rc == 0


if __name__ == '__main__':
    exit(main())
