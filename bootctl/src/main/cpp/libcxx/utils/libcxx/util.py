#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

from contextlib import contextmanager
import errno
import os
import platform
import signal
import subprocess
import sys
import tempfile
import threading


# FIXME: Most of these functions are cribbed from LIT
def to_bytes(str):
    # Encode to UTF-8 to get binary data.
    if isinstance(str, bytes):
        return str
    return str.encode('utf-8')

def to_string(bytes):
    if isinstance(bytes, str):
        return bytes
    return to_bytes(bytes)

def convert_string(bytes):
    try:
        return to_string(bytes.decode('utf-8'))
    except AttributeError: # 'str' object has no attribute 'decode'.
        return str(bytes)
    except UnicodeError:
        return str(bytes)


def cleanFile(filename):
    try:
        os.remove(filename)
    except OSError:
        pass


@contextmanager
def guardedTempFilename(suffix='', prefix='', dir=None):
    # Creates and yeilds a temporary filename within a with statement. The file
    # is removed upon scope exit.
    handle, name = tempfile.mkstemp(suffix=suffix, prefix=prefix, dir=dir)
    os.close(handle)
    yield name
    cleanFile(name)


@contextmanager
def guardedFilename(name):
    # yeilds a filename within a with statement. The file is removed upon scope
    # exit.
    yield name
    cleanFile(name)


@contextmanager
def nullContext(value):
    # yeilds a variable within a with statement. No action is taken upon scope
    # exit.
    yield value


def makeReport(cmd, out, err, rc):
    report = "Command: %s\n" % cmd
    report += "Exit Code: %d\n" % rc
    if out:
        report += "Standard Output:\n--\n%s--\n" % out
    if err:
        report += "Standard Error:\n--\n%s--\n" % err
    report += '\n'
    return report


def capture(args, env=None):
    """capture(command) - Run the given command (or argv list) in a shell and
    return the standard output. Raises a CalledProcessError if the command
    exits with a non-zero status."""
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         env=env)
    out, err = p.communicate()
    out = convert_string(out)
    err = convert_string(err)
    if p.returncode != 0:
        raise subprocess.CalledProcessError(cmd=args,
                                            returncode=p.returncode,
                                            output="{}\n{}".format(out, err))
    return out


def which(command, paths = None):
    """which(command, [paths]) - Look up the given command in the paths string
    (or the PATH environment variable, if unspecified)."""

    if paths is None:
        paths = os.environ.get('PATH','')

    # Check for absolute match first.
    if os.path.isfile(command):
        return command

    # Would be nice if Python had a lib function for this.
    if not paths:
        paths = os.defpath

    # Get suffixes to search.
    # On Cygwin, 'PATHEXT' may exist but it should not be used.
    if os.pathsep == ';':
        pathext = os.environ.get('PATHEXT', '').split(';')
    else:
        pathext = ['']

    # Search the paths...
    for path in paths.split(os.pathsep):
        for ext in pathext:
            p = os.path.join(path, command + ext)
            if os.path.exists(p) and not os.path.isdir(p):
                return p

    return None


def checkToolsPath(dir, tools):
    for tool in tools:
        if not os.path.exists(os.path.join(dir, tool)):
            return False
    return True


def whichTools(tools, paths):
    for path in paths.split(os.pathsep):
        if checkToolsPath(path, tools):
            return path
    return None

def mkdir_p(path):
    """mkdir_p(path) - Make the "path" directory, if it does not exist; this
    will also make directories for any missing parent directories."""
    if not path or os.path.exists(path):
        return

    parent = os.path.dirname(path)
    if parent != path:
        mkdir_p(parent)

    try:
        os.mkdir(path)
    except OSError:
        e = sys.exc_info()[1]
        # Ignore EEXIST, which may occur during a race condition.
        if e.errno != errno.EEXIST:
            raise


class ExecuteCommandTimeoutException(Exception):
    def __init__(self, msg, out, err, exitCode):
        assert isinstance(msg, str)
        assert isinstance(out, str)
        assert isinstance(err, str)
        assert isinstance(exitCode, int)
        self.msg = msg
        self.out = out
        self.err = err
        self.exitCode = exitCode

# Close extra file handles on UNIX (on Windows this cannot be done while
# also redirecting input).
kUseCloseFDs = not (platform.system() == 'Windows')
def executeCommand(command, cwd=None, env=None, input=None, timeout=0):
    """
        Execute command ``command`` (list of arguments or string)
        with
        * working directory ``cwd`` (str), use None to use the current
          working directory
        * environment ``env`` (dict), use None for none
        * Input to the command ``input`` (str), use string to pass
          no input.
        * Max execution time ``timeout`` (int) seconds. Use 0 for no timeout.

        Returns a tuple (out, err, exitCode) where
        * ``out`` (str) is the standard output of running the command
        * ``err`` (str) is the standard error of running the command
        * ``exitCode`` (int) is the exitCode of running the command

        If the timeout is hit an ``ExecuteCommandTimeoutException``
        is raised.
    """
    if input is not None:
        input = to_bytes(input)
    p = subprocess.Popen(command, cwd=cwd,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,
                         env=env, close_fds=kUseCloseFDs)
    timerObject = None
    # FIXME: Because of the way nested function scopes work in Python 2.x we
    # need to use a reference to a mutable object rather than a plain
    # bool. In Python 3 we could use the "nonlocal" keyword but we need
    # to support Python 2 as well.
    hitTimeOut = [False]
    try:
        if timeout > 0:
            def killProcess():
                # We may be invoking a shell so we need to kill the
                # process and all its children.
                hitTimeOut[0] = True
                killProcessAndChildren(p.pid)

            timerObject = threading.Timer(timeout, killProcess)
            timerObject.start()

        out,err = p.communicate(input=input)
        exitCode = p.wait()
    finally:
        if timerObject != None:
            timerObject.cancel()

    # Ensure the resulting output is always of string type.
    out = convert_string(out)
    err = convert_string(err)

    if hitTimeOut[0]:
        raise ExecuteCommandTimeoutException(
            msg='Reached timeout of {} seconds'.format(timeout),
            out=out,
            err=err,
            exitCode=exitCode
            )

    # Detect Ctrl-C in subprocess.
    if exitCode == -signal.SIGINT:
        raise KeyboardInterrupt

    return out, err, exitCode


def killProcessAndChildren(pid):
    """
    This function kills a process with ``pid`` and all its
    running children (recursively). It is currently implemented
    using the psutil module which provides a simple platform
    neutral implementation.

    TODO: Reimplement this without using psutil so we can
          remove our dependency on it.
    """
    import psutil
    try:
        psutilProc = psutil.Process(pid)
        # Handle the different psutil API versions
        try:
            # psutil >= 2.x
            children_iterator = psutilProc.children(recursive=True)
        except AttributeError:
            # psutil 1.x
            children_iterator = psutilProc.get_children(recursive=True)
        for child in children_iterator:
            try:
                child.kill()
            except psutil.NoSuchProcess:
                pass
        psutilProc.kill()
    except psutil.NoSuchProcess:
        pass


def executeCommandVerbose(cmd, *args, **kwargs):
    """
    Execute a command and print its output on failure.
    """
    out, err, exitCode = executeCommand(cmd, *args, **kwargs)
    if exitCode != 0:
        report = makeReport(cmd, out, err, exitCode)
        report += "\n\nFailed!"
        sys.stderr.write('%s\n' % report)
    return out, err, exitCode
