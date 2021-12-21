#! /usr/bin/env python
# encoding: utf-8

import argparse
import errno
import logging
import os
import platform
import re
import sys
import subprocess
import tempfile

try:
    import winreg
except ImportError:
    import _winreg as winreg
try:
    import urllib.request as request
except ImportError:
    import urllib as request
try:
    import urllib.parse as parse
except ImportError:
    import urlparse as parse

class EmptyLogger(object):
    '''
    Provides an implementation that performs no logging
    '''
    def debug(self, *k, **kw):
        pass
    def info(self, *k, **kw):
        pass
    def warn(self, *k, **kw):
        pass
    def error(self, *k, **kw):
        pass
    def critical(self, *k, **kw):
        pass
    def setLevel(self, *k, **kw):
        pass

urls = (
    'http://downloads.sourceforge.net/project/mingw-w64/Toolchains%20'
        'targetting%20Win32/Personal%20Builds/mingw-builds/installer/'
        'repository.txt',
    'http://downloads.sourceforge.net/project/mingwbuilds/host-windows/'
        'repository.txt'
)
'''
A list of mingw-build repositories
'''

def repository(urls = urls, log = EmptyLogger()):
    '''
    Downloads and parse mingw-build repository files and parses them
    '''
    log.info('getting mingw-builds repository')
    versions = {}
    re_sourceforge = re.compile(r'http://sourceforge.net/projects/([^/]+)/files')
    re_sub = r'http://downloads.sourceforge.net/project/\1'
    for url in urls:
        log.debug(' - requesting: %s', url)
        socket = request.urlopen(url)
        repo = socket.read()
        if not isinstance(repo, str):
            repo = repo.decode();
        socket.close()
        for entry in repo.split('\n')[:-1]:
            value = entry.split('|')
            version = tuple([int(n) for n in value[0].strip().split('.')])
            version = versions.setdefault(version, {})
            arch = value[1].strip()
            if arch == 'x32':
                arch = 'i686'
            elif arch == 'x64':
                arch = 'x86_64'
            arch = version.setdefault(arch, {})
            threading = arch.setdefault(value[2].strip(), {})
            exceptions = threading.setdefault(value[3].strip(), {})
            revision = exceptions.setdefault(int(value[4].strip()[3:]),
                re_sourceforge.sub(re_sub, value[5].strip()))
    return versions

def find_in_path(file, path=None):
    '''
    Attempts to find an executable in the path
    '''
    if platform.system() == 'Windows':
        file += '.exe'
    if path is None:
        path = os.environ.get('PATH', '')
    if type(path) is type(''):
        path = path.split(os.pathsep)
    return list(filter(os.path.exists,
        map(lambda dir, file=file: os.path.join(dir, file), path)))

def find_7zip(log = EmptyLogger()):
    '''
    Attempts to find 7zip for unpacking the mingw-build archives
    '''
    log.info('finding 7zip')
    path = find_in_path('7z')
    if not path:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\7-Zip')
        path, _ = winreg.QueryValueEx(key, 'Path')
        path = [os.path.join(path, '7z.exe')]
    log.debug('found \'%s\'', path[0])
    return path[0]

find_7zip()

def unpack(archive, location, log = EmptyLogger()):
    '''
    Unpacks a mingw-builds archive
    '''
    sevenzip = find_7zip(log)
    log.info('unpacking %s', os.path.basename(archive))
    cmd = [sevenzip, 'x', archive, '-o' + location, '-y']
    log.debug(' - %r', cmd)
    with open(os.devnull, 'w') as devnull:
        subprocess.check_call(cmd, stdout = devnull)

def download(url, location, log = EmptyLogger()):
    '''
    Downloads and unpacks a mingw-builds archive
    '''
    log.info('downloading MinGW')
    log.debug(' - url: %s', url)
    log.debug(' - location: %s', location)

    re_content = re.compile(r'attachment;[ \t]*filename=(")?([^"]*)(")?[\r\n]*')

    stream = request.urlopen(url)
    try:
        content = stream.getheader('Content-Disposition') or ''
    except AttributeError:
        content = stream.headers.getheader('Content-Disposition') or ''
    matches = re_content.match(content)
    if matches:
        filename = matches.group(2)
    else:
        parsed = parse.urlparse(stream.geturl())
        filename = os.path.basename(parsed.path)

    try:
        os.makedirs(location)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(location):
            pass
        else:
            raise

    archive = os.path.join(location, filename)
    with open(archive, 'wb') as out:
        while True:
            buf = stream.read(1024)
            if not buf:
                break
            out.write(buf)
    unpack(archive, location, log = log)
    os.remove(archive)

    possible = os.path.join(location, 'mingw64')
    if not os.path.exists(possible):
        possible = os.path.join(location, 'mingw32')
        if not os.path.exists(possible):
            raise ValueError('Failed to find unpacked MinGW: ' + possible)
    return possible

def root(location = None, arch = None, version = None, threading = None,
        exceptions = None, revision = None, log = EmptyLogger()):
    '''
    Returns the root folder of a specific version of the mingw-builds variant
    of gcc. Will download the compiler if needed
    '''

    # Get the repository if we don't have all the information
    if not (arch and version and threading and exceptions and revision):
        versions = repository(log = log)

    # Determine some defaults
    version = version or max(versions.keys())
    if not arch:
        arch = platform.machine().lower()
        if arch == 'x86':
            arch = 'i686'
        elif arch == 'amd64':
            arch = 'x86_64'
    if not threading:
        keys = versions[version][arch].keys()
        if 'posix' in keys:
            threading = 'posix'
        elif 'win32' in keys:
            threading = 'win32'
        else:
            threading = keys[0]
    if not exceptions:
        keys = versions[version][arch][threading].keys()
        if 'seh' in keys:
            exceptions = 'seh'
        elif 'sjlj' in keys:
            exceptions = 'sjlj'
        else:
            exceptions = keys[0]
    if revision == None:
        revision = max(versions[version][arch][threading][exceptions].keys())
    if not location:
        location = os.path.join(tempfile.gettempdir(), 'mingw-builds')

    # Get the download url
    url = versions[version][arch][threading][exceptions][revision]

    # Tell the user whatzzup
    log.info('finding MinGW %s', '.'.join(str(v) for v in version))
    log.debug(' - arch: %s', arch)
    log.debug(' - threading: %s', threading)
    log.debug(' - exceptions: %s', exceptions)
    log.debug(' - revision: %s', revision)
    log.debug(' - url: %s', url)

    # Store each specific revision differently
    slug = '{version}-{arch}-{threading}-{exceptions}-rev{revision}'
    slug = slug.format(
        version = '.'.join(str(v) for v in version),
        arch = arch,
        threading = threading,
        exceptions = exceptions,
        revision = revision
    )
    if arch == 'x86_64':
        root_dir = os.path.join(location, slug, 'mingw64')
    elif arch == 'i686':
        root_dir = os.path.join(location, slug, 'mingw32')
    else:
        raise ValueError('Unknown MinGW arch: ' + arch)

    # Download if needed
    if not os.path.exists(root_dir):
        downloaded = download(url, os.path.join(location, slug), log = log)
        if downloaded != root_dir:
            raise ValueError('The location of mingw did not match\n%s\n%s'
                % (downloaded, root_dir))

    return root_dir

def str2ver(string):
    '''
    Converts a version string into a tuple
    '''
    try:
        version = tuple(int(v) for v in string.split('.'))
        if len(version) is not 3:
            raise ValueError()
    except ValueError:
        raise argparse.ArgumentTypeError(
            'please provide a three digit version string')
    return version

def main():
    '''
    Invoked when the script is run directly by the python interpreter
    '''
    parser = argparse.ArgumentParser(
        description = 'Downloads a specific version of MinGW',
        formatter_class = argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('--location',
        help = 'the location to download the compiler to',
        default = os.path.join(tempfile.gettempdir(), 'mingw-builds'))
    parser.add_argument('--arch', required = True, choices = ['i686', 'x86_64'],
        help = 'the target MinGW architecture string')
    parser.add_argument('--version', type = str2ver,
        help = 'the version of GCC to download')
    parser.add_argument('--threading', choices = ['posix', 'win32'],
        help = 'the threading type of the compiler')
    parser.add_argument('--exceptions', choices = ['sjlj', 'seh', 'dwarf'],
        help = 'the method to throw exceptions')
    parser.add_argument('--revision', type=int,
        help = 'the revision of the MinGW release')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-v', '--verbose', action='store_true',
        help='increase the script output verbosity')
    group.add_argument('-q', '--quiet', action='store_true',
        help='only print errors and warning')
    args = parser.parse_args()

    # Create the logger
    logger = logging.getLogger('mingw')
    handler = logging.StreamHandler()
    formatter = logging.Formatter('%(message)s')
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    if args.quiet:
        logger.setLevel(logging.WARN)
    if args.verbose:
        logger.setLevel(logging.DEBUG)

    # Get MinGW
    root_dir = root(location = args.location, arch = args.arch,
        version = args.version, threading = args.threading,
        exceptions = args.exceptions, revision = args.revision,
        log = logger)

    sys.stdout.write('%s\n' % os.path.join(root_dir, 'bin'))

if __name__ == '__main__':
    try:
        main()
    except IOError as e:
        sys.stderr.write('IO error: %s\n' % e)
        sys.exit(1)
    except OSError as e:
        sys.stderr.write('OS error: %s\n' % e)
        sys.exit(1)
    except KeyboardInterrupt as e:
        sys.stderr.write('Killed\n')
        sys.exit(1)
