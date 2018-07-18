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
 * pts.h
 *
 * Manages the pseudo-terminal driver on Linux/Android and provides some
 * helper functions to handle raw input mode and terminal window resizing
 */

#ifndef _PTS_H_
#define _PTS_H_

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
int pts_open(char *slave_name, size_t slave_name_size);

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
int set_stdin_raw(void);

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
int restore_stdin(void);

/**
 * watch_sigwinch_async
 *
 * After calling this function, if the application receives
 * SIGWINCH, the terminal window size will be read from 
 * "input" and set on "output".
 *
 * NOTE: This function blocks SIGWINCH and spawns a thread.
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
int watch_sigwinch_async(int master, int slave);

/**
 * watch_sigwinch_cleanup
 *
 * Cause the SIGWINCH watcher thread to terminate
 */
void watch_sigwinch_cleanup(void);

/**
 * pump_stdin_async
 *
 * Forward data from STDIN to the given FD
 * in a seperate thread
 */
void pump_stdin_async(int outfd);

/**
 * pump_stdout_blocking
 *
 * Forward data from the FD to STDOUT.
 * Returns when the remote end of the FD closes.
 *
 * Before returning, restores stdin settings.
 */
void pump_stdout_blocking(int infd);

#endif
