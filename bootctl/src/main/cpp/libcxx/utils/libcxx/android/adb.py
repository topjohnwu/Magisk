import lit.util  # pylint: disable=import-error


class AdbError(RuntimeError):
    def __init__(self, cmd, out, err, exit_code):
        super(AdbError, self).__init__(err)
        self.cmd = cmd
        self.out = out
        self.err = err
        self.exit_code = exit_code


def mkdir(path):
    cmd = ['adb', 'shell', 'mkdir', path]
    out, err, exit_code = lit.util.executeCommand(cmd)
    if exit_code != 0:
        raise AdbError(cmd, out, err, exit_code)


def push(src, dst):
    cmd = ['adb', 'push', src, dst]
    out, err, exit_code = lit.util.executeCommand(cmd)
    if exit_code != 0:
        raise AdbError(cmd, out, err, exit_code)
