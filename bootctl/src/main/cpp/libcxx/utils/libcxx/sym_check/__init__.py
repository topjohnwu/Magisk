#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

"""libcxx abi symbol checker"""

__author__ = 'Eric Fiselier'
__email__ = 'eric@efcs.ca'
__versioninfo__ = (0, 1, 0)
__version__ = ' '.join(str(v) for v in __versioninfo__) + 'dev'

__all__ = ['diff', 'extract', 'util']
