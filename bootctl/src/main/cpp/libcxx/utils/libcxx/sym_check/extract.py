# -*- Python -*- vim: set syntax=python tabstop=4 expandtab cc=80:
#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##
"""
extract - A set of function that extract symbol lists from shared libraries.
"""
import distutils.spawn
import sys
import re

import libcxx.util
from libcxx.sym_check import util

extract_ignore_names = ['_init', '_fini']

class NMExtractor(object):
    """
    NMExtractor - Extract symbol lists from libraries using nm.
    """

    @staticmethod
    def find_tool():
        """
        Search for the nm executable and return the path.
        """
        return distutils.spawn.find_executable('nm')

    def __init__(self):
        """
        Initialize the nm executable and flags that will be used to extract
        symbols from shared libraries.
        """
        self.nm_exe = self.find_tool()
        if self.nm_exe is None:
            # ERROR no NM found
            print("ERROR: Could not find nm")
            sys.exit(1)
        self.flags = ['-P', '-g']

    def extract(self, lib):
        """
        Extract symbols from a library and return the results as a dict of
        parsed symbols.
        """
        cmd = [self.nm_exe] + self.flags + [lib]
        out, _, exit_code = libcxx.util.executeCommandVerbose(cmd)
        if exit_code != 0:
            raise RuntimeError('Failed to run %s on %s' % (self.nm_exe, lib))
        fmt_syms = (self._extract_sym(l)
                    for l in out.splitlines() if l.strip())
            # Cast symbol to string.
        final_syms = (repr(s) for s in fmt_syms if self._want_sym(s))
        # Make unique and sort strings.
        tmp_list = list(sorted(set(final_syms)))
        # Cast string back to symbol.
        return util.read_syms_from_list(tmp_list)

    def _extract_sym(self, sym_str):
        bits = sym_str.split()
        # Everything we want has at least two columns.
        if len(bits) < 2:
            return None
        new_sym = {
            'name': bits[0],
            'type': bits[1],
            'is_defined': (bits[1].lower() != 'u')
        }
        new_sym['name'] = new_sym['name'].replace('@@', '@')
        new_sym = self._transform_sym_type(new_sym)
        # NM types which we want to save the size for.
        if new_sym['type'] == 'OBJECT' and len(bits) > 3:
            new_sym['size'] = int(bits[3], 16)
        return new_sym

    @staticmethod
    def _want_sym(sym):
        """
        Check that s is a valid symbol that we want to keep.
        """
        if sym is None or len(sym) < 2:
            return False
        if sym['name'] in extract_ignore_names:
            return False
        bad_types = ['t', 'b', 'r', 'd', 'w']
        return (sym['type'] not in bad_types
                and sym['name'] not in ['__bss_start', '_end', '_edata'])

    @staticmethod
    def _transform_sym_type(sym):
        """
        Map the nm single letter output for type to either FUNC or OBJECT.
        If the type is not recognized it is left unchanged.
        """
        func_types = ['T', 'W']
        obj_types = ['B', 'D', 'R', 'V', 'S']
        if sym['type'] in func_types:
            sym['type'] = 'FUNC'
        elif sym['type'] in obj_types:
            sym['type'] = 'OBJECT'
        return sym

class ReadElfExtractor(object):
    """
    ReadElfExtractor - Extract symbol lists from libraries using readelf.
    """

    @staticmethod
    def find_tool():
        """
        Search for the readelf executable and return the path.
        """
        return distutils.spawn.find_executable('readelf')

    def __init__(self):
        """
        Initialize the readelf executable and flags that will be used to
        extract symbols from shared libraries.
        """
        self.tool = self.find_tool()
        if self.tool is None:
            # ERROR no NM found
            print("ERROR: Could not find readelf")
            sys.exit(1)
        self.flags = ['--wide', '--symbols']

    def extract(self, lib):
        """
        Extract symbols from a library and return the results as a dict of
        parsed symbols.
        """
        cmd = [self.tool] + self.flags + [lib]
        out, _, exit_code = libcxx.util.executeCommandVerbose(cmd)
        if exit_code != 0:
            raise RuntimeError('Failed to run %s on %s' % (self.nm_exe, lib))
        dyn_syms = self.get_dynsym_table(out)
        return self.process_syms(dyn_syms)

    def process_syms(self, sym_list):
        new_syms = []
        for s in sym_list:
            parts = s.split()
            if not parts:
                continue
            assert len(parts) == 7 or len(parts) == 8 or len(parts) == 9
            if len(parts) == 7:
                continue
            new_sym = {
                'name': parts[7],
                'size': int(parts[2]),
                'type': parts[3],
                'is_defined': (parts[6] != 'UND')
            }
            assert new_sym['type'] in ['OBJECT', 'FUNC', 'NOTYPE']
            if new_sym['name'] in extract_ignore_names:
                continue
            if new_sym['type'] == 'NOTYPE':
                continue
            if new_sym['type'] == 'FUNC':
                del new_sym['size']
            new_syms += [new_sym]
        return new_syms

    def get_dynsym_table(self, out):
        lines = out.splitlines()
        start = -1
        end = -1
        for i in range(len(lines)):
            if lines[i].startswith("Symbol table '.dynsym'"):
                start = i + 2
            if start != -1 and end == -1 and not lines[i].strip():
                end = i + 1
        assert start != -1
        if end == -1:
            end = len(lines)
        return lines[start:end]


def extract_symbols(lib_file):
    """
    Extract and return a list of symbols extracted from a dynamic library.
    The symbols are extracted using NM. They are then filtered and formated.
    Finally they symbols are made unique.
    """
    if ReadElfExtractor.find_tool():
        extractor = ReadElfExtractor()
    else:
        extractor = NMExtractor()
    return extractor.extract(lib_file)
