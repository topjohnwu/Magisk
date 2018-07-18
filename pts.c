/*
 * Copyright 2013, Tan Chee Eng (@tan-ce)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /*
 * pts.c
 *
 * Manages the pseudo-terminal driver on Linux/Android and provides some
 * helper functions to handle raw input mode and terminal window resizing
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>

#include "pts.h"

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
static void pump_ex(int input, int output, int close_output) {
    char buf[4096];
    int len;
    while ((len = read(input, buf, 4096)) > 0) {
        if (write_blocking(output, buf, len) == -1) break;
    }
    close(input);
    if (close_output) close(output);
}

/**
 * Pump data from input FD to output FD. Will close the
 * output FD when done.
 */
static void pump(int input, int output) {
    pump_ex(input, output, 1);
}

static void* pump_thread(void* data) {
    int* files = (int*)data;
    int input = files[0];
    int output = files[1];
    pump(input, output);
    free(data);
    return NULL;
}

static void pump_async(int input, int output) {
    pthread_t writer;
    int* files = (int*)malloc(sizeof(int) * 2);
    if (files == NULL) {
        exit(-1);
    }
    files[0] = input;
    files[1] = output;
    pthread_create(&writer, NULL, pump_thread, files);
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
    char sn_tmp[256];

    // Open master ptmx device
    fdm = open("/dev/ptmx", O_RDWR);
    if (fdm == -1) return -1;

    // Get the slave name
    if (ptsname_r(fdm, slave_name, slave_name_size-1)) {
        close(fdm);
        return -2;
    }

    slave_name[slave_name_size - 1] = '\0';

    // Grant, then unlock
    if (grantpt(fdm) == -1) {
        close(fdm);
        return -1;
    }
    if (unlockpt(fdm) == -1) {
        close(fdm);
        return -1;
    }

    return fdm;
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
int set_stdin_raw(void) {
    struct termios new_termios;

    // Save the current stdin termios
    if (tcgetattr(STDIN_FILENO, &old_stdin) < 0) {
        return -1;
    }

    // Start from the current settings
    new_termios = old_stdin;

    // Make the terminal like an SSH or telnet client
    new_termios.c_iflag |= IGNPAR;
    new_termios.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXANY | IXOFF);
    new_termios.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL);
    new_termios.c_oflag &= ~OPOST;
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) < 0) {
        return -1;
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
int restore_stdin(void) {
    if (!stdin_is_raw) return 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_stdin) < 0) {
        return -1;
    }

    stdin_is_raw = 0;

    return 0;
}

// Flag indicating whether the sigwinch watcher should terminate.
static volatile int closing_time = 0;

/**
 * Thread process. Wait for a SIGWINCH to be received, then update 
 * the terminal size.
 */
static void *watch_sigwinch(void *data) {
    sigset_t winch;
    int sig;
    int master = ((int *)data)[0];
    int slave = ((int *)data)[1];

    sigemptyset(&winch);
    sigaddset(&winch, SIGWINCH);

    do {
        // Wait for a SIGWINCH
        sigwait(&winch, &sig);

        if (closing_time) break;

        // Get the new terminal size
        struct winsize w;
        if (ioctl(master, TIOCGWINSZ, &w) == -1) {
            continue;
        }

        // Set the new terminal size
        ioctl(slave, TIOCSWINSZ, &w);

    } while (1);

    free(data);
    return NULL;
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
    int *files = (int *) malloc(sizeof(int) * 2);
    if (files == NULL) {
        return -1;
    }

    // Block SIGWINCH so sigwait can later receive it
    sigset_t winch;
    sigemptyset(&winch);
    sigaddset(&winch, SIGWINCH);
    if (sigprocmask(SIG_BLOCK, &winch, NULL) == -1) {
        free(files);
        return -1;
    }

    // Initialize some variables, then start the thread
    closing_time = 0;
    files[0] = master;
    files[1] = slave;
    int ret = pthread_create(&watcher, NULL, &watch_sigwinch, files);
    if (ret != 0) {
        free(files);
        errno = ret;
        return -1;
    }

    // Set the initial terminal size
    raise(SIGWINCH);
    return 0;
}

/**
 * watch_sigwinch_cleanup
 *
 * Cause the SIGWINCH watcher thread to terminate
 */
void watch_sigwinch_cleanup(void) {
    closing_time = 1;
    raise(SIGWINCH);
}

/**
 * pump_stdin_async
 *
 * Forward data from STDIN to the given FD
 * in a seperate thread
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
    pump_ex(infd, STDOUT_FILENO, 0 /* Don't close output when done */);

    // Cleanup
    restore_stdin();
    watch_sigwinch_cleanup();
}
