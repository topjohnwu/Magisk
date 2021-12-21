import sys
import os
import socket
import stat

# Ensure that this is being run on a specific platform
assert sys.platform.startswith('linux') or sys.platform.startswith('darwin') \
    or sys.platform.startswith('cygwin') or sys.platform.startswith('freebsd') \
    or sys.platform.startswith('netbsd')

def env_path():
    ep = os.environ.get('LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT')
    assert ep is not None
    ep = os.path.realpath(ep)
    assert os.path.isdir(ep)
    return ep

env_path_global = env_path()

# Make sure we don't try and write outside of env_path.
# All paths used should be sanitized
def sanitize(p):
    p = os.path.realpath(p)
    if os.path.commonprefix([env_path_global, p]):
        return p
    assert False

"""
Some of the tests restrict permissions to induce failures.
Before we delete the test environment, we have to walk it and re-raise the
permissions.
"""
def clean_recursive(root_p):
    if not os.path.islink(root_p):
        os.chmod(root_p, 0o777)
    for ent in os.listdir(root_p):
        p = os.path.join(root_p, ent)
        if os.path.islink(p) or not os.path.isdir(p):
            os.remove(p)
        else:
            assert os.path.isdir(p)
            clean_recursive(p)
            os.rmdir(p)


def init_test_directory(root_p):
    root_p = sanitize(root_p)
    assert not os.path.exists(root_p)
    os.makedirs(root_p)


def destroy_test_directory(root_p):
    root_p = sanitize(root_p)
    clean_recursive(root_p)
    os.rmdir(root_p)


def create_file(fname, size):
    with open(sanitize(fname), 'w') as f:
        f.write('c' * size)


def create_dir(dname):
    os.mkdir(sanitize(dname))


def create_symlink(source, link):
    os.symlink(sanitize(source), sanitize(link))


def create_hardlink(source, link):
    os.link(sanitize(source), sanitize(link))


def create_fifo(source):
    os.mkfifo(sanitize(source))


def create_socket(source):
    sock = socket.socket(socket.AF_UNIX)
    sanitized_source = sanitize(source)
    # AF_UNIX sockets may have very limited path length, so split it
    # into chdir call (with technically unlimited length) followed
    # by bind() relative to the directory
    os.chdir(os.path.dirname(sanitized_source))
    sock.bind(os.path.basename(sanitized_source))


if __name__ == '__main__':
    command = " ".join(sys.argv[1:])
    eval(command)
    sys.exit(0)
