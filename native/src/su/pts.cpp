/*
 * Copyright 2013, Tan Chee Eng (@tan-ce)
 */

 /*
 * pts.c
 *
 * Manages the pseudo-terminal driver on Linux/Android and provides some
 * helper functions to handle raw input mode and terminal window resizing
 */

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <base.hpp>

#include "pts.hpp"

/**
 * Helper functions
 */
// Ensures all the data is written out
static int write_blocking(int fd, char *buf, ssize_t bufsz) {
    ssize_t ret, written;

    written = 0;
    do {
        ret = write(fd, buf + written, bufsz - written);
        if (ret == -1) return -1;
        written += ret;
    } while (written < bufsz);

    return 0;
}

/**
 * Pump data from input FD to output FD. If close_output is
 * true, then close the output FD when we're done.
 */
static void pump(int input, int output, bool close_output = true) {
    char buf[4096];
    int len;
    while ((len = read(input, buf, 4096)) > 0) {
        if (write_blocking(output, buf, len) == -1) break;
    }
    close(input);
    if (close_output) close(output);
}

static void* pump_thread(void* data) {
    int *fds = (int*) data;
    pump(fds[0], fds[1]);
    delete[] fds;
    return nullptr;
}

static void pump_async(int input, int output) {
    pthread_t writer;
    int *fds = new int[2];
    fds[0] = input;
    fds[1] = output;
    pthread_create(&writer, nullptr, pump_thread, fds);
}


/**
 * pts_open
 *
 * Opens a pts device and returns the name of the slave tty device.
 *
 * Arguments
 * slave_name       the name of the slave device
 * slave_name_size  the size of the buffer passed via slave_name
 *
 * Return Values
 * on failure either -2 or -1 (errno set) is returned.
 * on success, the file descriptor of the master device is returned.
 */
int pts_open(char *slave_name, size_t slave_name_size) {
    int fdm;

    // Open master ptmx device
    fdm = open("/dev/ptmx", O_RDWR);
    if (fdm == -1)
        goto error;

    // Get the slave name
    if (ptsname_r(fdm, slave_name, slave_name_size - 1))
        goto error;

    slave_name[slave_name_size - 1] = '\0';

    // Grant, then unlock
    if (grantpt(fdm) == -1)
        goto error;

    if (unlockpt(fdm) == -1)
        goto error;

    return fdm;
error:
    close(fdm);
    PLOGE("pts_open");
    return -1;
}

int get_pty_num(int fd) {
    int pty_num = -1;
    if (ioctl(fd, TIOCGPTN, &pty_num) != 0) {
        LOGW("get_pty_num failed with %d: %s\n", errno, std::strerror(errno));
        return -1;
    }
    return pty_num;
}

// Stores the previous termios of stdin
static struct termios old_stdin;
static int stdin_is_raw = 0;

/**
 * set_stdin_raw
 *
 * Changes stdin to raw unbuffered mode, disables echo,
 * auto carriage return, etc.
 *
 * Return Value
 * on failure -1, and errno is set
 * on success 0
 */
int set_stdin_raw() {
    struct termios termios{};

    if (tcgetattr(STDIN_FILENO, &termios) < 0) {
        return -1;
    }

    old_stdin = termios;

    cfmakeraw(&termios);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios) < 0) {
        // https://blog.zhanghai.me/fixing-line-editing-on-android-8-0/
        if (tcsetattr(STDIN_FILENO, TCSADRAIN, &termios) < 0) {
            return -1;
        }
    }

    stdin_is_raw = 1;

    return 0;
}

/**
 * restore_stdin
 *
 * Restore termios on stdin to the state it was before
 * set_stdin_raw() was called. If set_stdin_raw() was
 * never called, does nothing and doesn't return an error.
 *
 * This function is async-safe.
 *
 * Return Value
 * on failure, -1 and errno is set
 * on success, 0
 */
int restore_stdin() {
    if (!stdin_is_raw) return 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_stdin) < 0) {
        if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_stdin) < 0) {
            return -1;
        }
    }

    stdin_is_raw = 0;

    return 0;
}

// Flag indicating whether the sigwinch watcher should terminate.
static volatile bool close_sigwinch_watcher = false;

/**
 * Thread process. Wait for a SIGWINCH to be received, then update
 * the terminal size.
 */
static void *watch_sigwinch(void *data) {
    sigset_t winch;
    int *fds = (int *)data;
    int sig;

    sigemptyset(&winch);
    sigaddset(&winch, SIGWINCH);
    pthread_sigmask(SIG_UNBLOCK, &winch, nullptr);

    do {
        if (close_sigwinch_watcher)
            break;

        // Get the new terminal size
        struct winsize w;
        if (ioctl(fds[0], TIOCGWINSZ, &w) == -1)
            continue;

        // Set the new terminal size
        ioctl(fds[1], TIOCSWINSZ, &w);

    } while (sigwait(&winch, &sig) == 0);
    delete[] fds;

    return nullptr;
}

/**
 * watch_sigwinch_async
 *
 * After calling this function, if the application receives
 * SIGWINCH, the terminal window size will be read from
 * "input" and set on "output".
 *
 * NOTE: This function blocks SIGWINCH and spawns a thread.
 * NOTE 2: This function must be called before any of the
 *         pump functions.
 *
 * Arguments
 * master   A file descriptor of the TTY window size to follow
 * slave    A file descriptor of the TTY window size which is
 *          to be set on SIGWINCH
 *
 * Return Value
 * on failure, -1 and errno will be set. In this case, no
 *      thread has been spawned and SIGWINCH will not be
 *      blocked.
 * on success, 0
 */
int watch_sigwinch_async(int master, int slave) {
    pthread_t watcher;
    int *fds = new int[2];

    // Block SIGWINCH so sigwait can later receive it
    sigset_t winch;
    sigemptyset(&winch);
    sigaddset(&winch, SIGWINCH);
    if (pthread_sigmask(SIG_BLOCK, &winch, nullptr) == -1) {
        delete[] fds;
        return -1;
    }

    // Initialize some variables, then start the thread
    close_sigwinch_watcher = 0;
    fds[0] = master;
    fds[1] = slave;
    int ret = pthread_create(&watcher, nullptr, &watch_sigwinch, fds);
    if (ret != 0) {
        delete[] fds;
        errno = ret;
        return -1;
    }

    return 0;
}

/**
 * pump_stdin_async
 *
 * Forward data from STDIN to the given FD
 * in a separate thread
 */
void pump_stdin_async(int outfd) {
    // Put stdin into raw mode
    set_stdin_raw();

    // Pump data from stdin to the PTY
    pump_async(STDIN_FILENO, outfd);
}

/**
 * pump_stdout_blocking
 *
 * Forward data from the FD to STDOUT.
 * Returns when the remote end of the FD closes.
 *
 * Before returning, restores stdin settings.
 */
void pump_stdout_blocking(int infd) {
    // Pump data from stdout to PTY
    pump(infd, STDOUT_FILENO, false /* Don't close output when done */);

    // Cleanup
    restore_stdin();
    close_sigwinch_watcher = true;
    raise(SIGWINCH);
}
