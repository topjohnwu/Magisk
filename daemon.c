/*
** Copyright 2010, Adam Shanks (@ChainsDD)
** Copyright 2008, Zinx Verituse (@zinxv)
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define _GNU_SOURCE /* for unshare() */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <pwd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <selinux/selinux.h>

#ifdef SUPERUSER_EMBEDDED
#include <cutils/multiuser.h>
#endif

#include "binds.h"
#include "su.h"
#include "utils.h"
#include "pts.h"

int is_daemon = 0;
int daemon_from_uid = 0;
int daemon_from_pid = 0;

// Constants for the atty bitfield
#define ATTY_IN     1
#define ATTY_OUT    2
#define ATTY_ERR    4

/*
 * Receive a file descriptor from a Unix socket.
 * Contributed by @mkasick
 *
 * Returns the file descriptor on success, or -1 if a file
 * descriptor was not actually included in the message
 *
 * On error the function terminates by calling exit(-1)
 */
static int recv_fd(int sockfd) {
    // Need to receive data from the message, otherwise don't care about it.
    char iovbuf;

    struct iovec iov = {
        .iov_base = &iovbuf,
        .iov_len  = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
        .msg_control    = cmsgbuf,
        .msg_controllen = sizeof(cmsgbuf),
    };

    if (recvmsg(sockfd, &msg, MSG_WAITALL) != 1) {
        goto error;
    }

    // Was a control message actually sent?
    switch (msg.msg_controllen) {
    case 0:
        // No, so the file descriptor was closed and won't be used.
        return -1;
    case sizeof(cmsgbuf):
        // Yes, grab the file descriptor from it.
        break;
    default:
        goto error;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    if (cmsg             == NULL                  ||
        cmsg->cmsg_len   != CMSG_LEN(sizeof(int)) ||
        cmsg->cmsg_level != SOL_SOCKET            ||
        cmsg->cmsg_type  != SCM_RIGHTS) {
error:
        LOGE("unable to read fd");
        exit(-1);
    }

    return *(int *)CMSG_DATA(cmsg);
}

/*
 * Send a file descriptor through a Unix socket.
 * Contributed by @mkasick
 *
 * On error the function terminates by calling exit(-1)
 *
 * fd may be -1, in which case the dummy data is sent,
 * but no control message with the FD is sent.
 */
static void send_fd(int sockfd, int fd) {
    // Need to send some data in the message, this will do.
    struct iovec iov = {
        .iov_base = "",
        .iov_len  = 1,
    };

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    if (fd != -1) {
        // Is the file descriptor actually open?
        if (fcntl(fd, F_GETFD) == -1) {
            if (errno != EBADF) {
                goto error;
            }
            // It's closed, don't send a control message or sendmsg will EBADF.
        } else {
            // It's open, send the file descriptor in a control message.
            msg.msg_control    = cmsgbuf;
            msg.msg_controllen = sizeof(cmsgbuf);

            struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

            cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type  = SCM_RIGHTS;

            *(int *)CMSG_DATA(cmsg) = fd;
        }
    }

    if (sendmsg(sockfd, &msg, 0) != 1) {
error:
        PLOGE("unable to send fd");
        exit(-1);
    }
}

static int read_int(int fd) {
    int val;
    int len = read(fd, &val, sizeof(int));
    if (len != sizeof(int)) {
        LOGE("unable to read int: %d", len);
        exit(-1);
    }
    return val;
}

static void write_int(int fd, int val) {
    int written = write(fd, &val, sizeof(int));
    if (written != sizeof(int)) {
        PLOGE("unable to write int");
        exit(-1);
    }
}

static char* read_string(int fd) {
    int len = read_int(fd);
    if (len > PATH_MAX || len < 0) {
        LOGE("invalid string length %d", len);
        exit(-1);
    }
    char* val = malloc(sizeof(char) * (len + 1));
    if (val == NULL) {
        LOGE("unable to malloc string");
        exit(-1);
    }
    val[len] = '\0';
    int amount = read(fd, val, len);
    if (amount != len) {
        LOGE("unable to read string");
        exit(-1);
    }
    return val;
}

static void write_string(int fd, char* val) {
    int len = strlen(val);
    write_int(fd, len);
    int written = write(fd, val, len);
    if (written != len) {
        PLOGE("unable to write string");
        exit(-1);
    }
}

#ifdef SUPERUSER_EMBEDDED
static void mount_emulated_storage(int user_id) {
    const char *emulated_source = getenv("EMULATED_STORAGE_SOURCE");
    const char *emulated_target = getenv("EMULATED_STORAGE_TARGET");
    const char* legacy = getenv("EXTERNAL_STORAGE");

    if (!emulated_source || !emulated_target) {
        // No emulated storage is present
        return;
    }

    // Create a second private mount namespace for our process
    if (unshare(CLONE_NEWNS) < 0) {
        PLOGE("unshare");
        return;
    }

    if (mount("rootfs", "/", NULL, MS_SLAVE | MS_REC, NULL) < 0) {
        PLOGE("mount rootfs as slave");
        return;
    }

    // /mnt/shell/emulated -> /storage/emulated
    if (mount(emulated_source, emulated_target, NULL, MS_BIND, NULL) < 0) {
        PLOGE("mount emulated storage");
    }

    char target_user[PATH_MAX];
    snprintf(target_user, PATH_MAX, "%s/%d", emulated_target, user_id);

    // /mnt/shell/emulated/<user> -> /storage/emulated/legacy
    if (mount(target_user, legacy, NULL, MS_BIND | MS_REC, NULL) < 0) {
        PLOGE("mount legacy path");
    }
}
#endif

static int run_daemon_child(int infd, int outfd, int errfd, int argc, char** argv) {
    if (-1 == dup2(outfd, STDOUT_FILENO)) {
        PLOGE("dup2 child outfd");
        exit(-1);
    }

    if (-1 == dup2(errfd, STDERR_FILENO)) {
        PLOGE("dup2 child errfd");
        exit(-1);
    }

    if (-1 == dup2(infd, STDIN_FILENO)) {
        PLOGE("dup2 child infd");
        exit(-1);
    }

    close(infd);
    close(outfd);
    close(errfd);

    return su_main_nodaemon(argc, argv);
}

static int daemon_accept(int fd) {
    is_daemon = 1;
    int pid = read_int(fd);
    LOGD("remote pid: %d", pid);
    char *pts_slave = read_string(fd);
    LOGD("remote pts_slave: %s", pts_slave);
    daemon_from_uid = read_int(fd);
    LOGD("remote uid: %d", daemon_from_uid);
    daemon_from_pid = read_int(fd);
    LOGD("remote req pid: %d", daemon_from_pid);

    struct ucred credentials;
    int ucred_length = sizeof(struct ucred);
    /* fill in the user data structure */
    if(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length)) {
        LOGE("could obtain credentials from unix domain socket");
        exit(-1);
    }
    // if the credentials on the other side of the wire are NOT root,
    // we can't trust anything being sent.
    if (credentials.uid != 0) {
        daemon_from_uid = credentials.uid;
        pid = credentials.pid;
        daemon_from_pid = credentials.pid;
    }

    int mount_storage = read_int(fd);
    // The the FDs for each of the streams
    int infd  = recv_fd(fd);
    int outfd = recv_fd(fd);
    int errfd = recv_fd(fd);

    int argc = read_int(fd);
    if (argc < 0 || argc > 512) {
        LOGE("unable to allocate args: %d", argc);
        exit(-1);
    }
    LOGD("remote args: %d", argc);
    char** argv = (char**)malloc(sizeof(char*) * (argc + 1));
    argv[argc] = NULL;
    int i;
    for (i = 0; i < argc; i++) {
        argv[i] = read_string(fd);
    }

    // ack
    write_int(fd, 1);

    // Fork the child process. The fork has to happen before calling
    // setsid() and opening the pseudo-terminal so that the parent
    // is not affected
    int child = fork();
    if (child < 0) {
        // fork failed, send a return code and bail out
        PLOGE("unable to fork");
        write(fd, &child, sizeof(int));
        close(fd);
        return child;
    }

    if (child != 0) {
        // In parent, wait for the child to exit, and send the exit code
        // across the wire.
        int status, code;

        free(pts_slave);

        LOGD("waiting for child exit");
        if (waitpid(child, &status, 0) > 0) {
            code = WEXITSTATUS(status);
        }
        else {
            code = -1;
        }

        // Pass the return code back to the client
        LOGD("sending code");
        if (write(fd, &code, sizeof(int)) != sizeof(int)) {
            PLOGE("unable to write exit code");
        }

        close(fd);
        LOGD("child exited");
        return code;
    }

    // We are in the child now
    // Close the unix socket file descriptor
    close (fd);

    // Become session leader
    if (setsid() == (pid_t) -1) {
        PLOGE("setsid");
    }

    int ptsfd;
    if (pts_slave[0]) {
		//Check pts_slave file is owned by daemon_from_uid
		{
			struct stat stbuf;
			int res = stat(pts_slave, &stbuf);
			if(res) {
				PLOGE("stat(pts_slave) daemon");
				exit(-1);
			}

			//If caller is not root, ensure the owner of pts_slave is the caller
			if(stbuf.st_uid != credentials.uid &&
					credentials.uid != 0) {
				PLOGE("Wrong permission of pts_slave");
				exit(-1);
			}
		}

        // Opening the TTY has to occur after the
        // fork() and setsid() so that it becomes
        // our controlling TTY and not the daemon's
        ptsfd = open(pts_slave, O_RDWR);
        if (ptsfd == -1) {
            PLOGE("open(pts_slave) daemon");
            exit(-1);
        }

		//Check we haven't been fooled
		{
			struct stat stbuf;
			int res = fstat(ptsfd, &stbuf);
			if(res) {
				//If we have been fooled DO NOT WRITE ANYTHING
				_exit(2);
			}

			if(stbuf.st_uid != credentials.uid &&
					credentials.uid != 0) {
				_exit(2);
			}
		}

        if (infd < 0)  {
            LOGD("daemon: stdin using PTY");
            infd  = ptsfd;
        }
        if (outfd < 0) {
            LOGD("daemon: stdout using PTY");
            outfd = ptsfd;
        }
        if (errfd < 0) {
            LOGD("daemon: stderr using PTY");
            errfd = ptsfd;
        }
    } else {
        // If a TTY was sent directly, make it the CTTY.
        if (isatty(infd)) {
            ioctl(infd, TIOCSCTTY, 1);
        }
    }
    free(pts_slave);

#ifdef SUPERUSER_EMBEDDED
    if (mount_storage) {
        mount_emulated_storage(multiuser_get_user_id(daemon_from_uid));
    }
#endif

    return run_daemon_child(infd, outfd, errfd, argc, argv);
}

static int copy_file(const char* src, const char* dst, int mode) {
	int ifd = open(src, O_RDONLY);
	if(ifd<0)
		return 1;
	if(mode == 0) {
		struct stat stbuf;
		if(fstat(ifd, &stbuf))
			return 1;
		mode = stbuf.st_mode & 0777;
		LOGE("File %s found mode %o", src, mode);

	}
	int ofd = open(dst, O_WRONLY|O_CREAT, mode);
	if(ofd<0)
		return 1;
	size_t s = lseek(ifd, 0, SEEK_END);
	if(s<0)
		return 1;
	lseek(ifd, 0, SEEK_SET);
	int ret = sendfile(ofd, ifd, NULL, s);
	if(ret<0)
		return 1;
	close(ofd);
	close(ifd);
	return 0;
}

static void prepare_su_bind() {
	int ret = 0;

	//Check if there is a use to mount bind
	if(access("/system/xbin/su", R_OK) != 0)
		return;

	ret = copy_file("/sbin/su", "/dev/su/su", 0755);
	if(ret) {
		PLOGE("Failed to copy su");
		return;
	}
	chmod("/dev/su/su", 0755);

	ret = setfilecon("/dev/su/su", "u:object_r:system_file:s0");
	if(ret) {
		LOGE("Failed to set file context");
		return;
	}

	ret = mount("/dev/su/su", "/system/xbin/su", "", MS_BIND, NULL);
	if(ret) {
		LOGE("Failed to mount bind");
		return;
	}
}

static void prepare_binds() {
	mkdir("/data/su", 0700);
	static int i = 0;

	auto void cb(void *arg, int uid, const char *src, const char *dst) {
		int ret = 0;

		char *tmpfile = NULL;
		asprintf(&tmpfile, "/dev/su/bind%d", i++);
		struct stat stbuf;
		ret = stat(src, &stbuf);
		if(ret) {
			free(tmpfile);
			LOGE("Failed to stat src %s file", src);
			return;
		}

		//Only shell uid is allowed to bind files not his own
		if(uid != 2000 && uid != stbuf.st_uid) {
			LOGE("File %s has wrong owner: %d vs %d", src, uid, stbuf.st_uid);
			return;
		}

		ret = copy_file(src, tmpfile, 0);
		if(ret) {
			free(tmpfile);
			PLOGE("Failed to copy su");
			return;
		}
		chmod(tmpfile, stbuf.st_mode);

		ret = setfilecon(tmpfile, "u:object_r:system_file:s0");
		if(ret) {
			LOGE("Failed to set file context");
			return;
		}

		ret = mount(tmpfile, dst, "", MS_BIND, NULL);
		if(ret) {
			LOGE("Failed to mount bind");
			return;
		}
	}
	bind_foreach(cb, NULL);
}

static void do_init() {
	auto void cb(void *arg, int uid, const char *path) {
		int ret = 0;

		int p = fork();
		if(p)
			return;

		while(access("/system/bin/sh", R_OK)) sleep(1);
		ret = setexeccon("u:r:su:s0");
		execl(path, path, NULL);
		LOGE("Failed to execute %s. Trying as shell script, ret = %d", path, ret);

		ret = setexeccon("u:r:su:s0");
		execl("/system/bin/sh", "/system/bin/sh", path, NULL);
		LOGE("Failed to execute %s as shell script", path);
		_exit(1);
	}
	init_foreach(cb, NULL);
}

static void prepare() {
	setfscreatecon("u:object_r:su_daemon:s0");
	mkdir("/dev/su", 0700);
	prepare_su_bind();
	prepare_binds();
	do_init();
	setfscreatecon(NULL);
}

int run_daemon() {
    if (getuid() != 0 || getgid() != 0) {
        PLOGE("daemon requires root. uid/gid not root");
        return -1;
    }

	prepare();

    int fd;
    struct sockaddr_un sun;

    fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0) {
        PLOGE("socket");
        return -1;
    }
    if (fcntl(fd, F_SETFD, FD_CLOEXEC)) {
        PLOGE("fcntl FD_CLOEXEC");
        goto err;
    }

    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_LOCAL;
    sprintf(sun.sun_path, "%s/server", REQUESTOR_DAEMON_PATH);

    /*
     * Delete the socket to protect from situations when
     * something bad occured previously and the kernel reused pid from that process.
     * Small probability, isn't it.
     */
    unlink(sun.sun_path);
    unlink(REQUESTOR_DAEMON_PATH);

    int previous_umask = umask(027);
    mkdir(REQUESTOR_DAEMON_PATH, 0777);

    memset(sun.sun_path, 0, sizeof(sun.sun_path));
    memcpy(sun.sun_path, "\0" "SUPERUSER", strlen("SUPERUSER") + 1);

    if (bind(fd, (struct sockaddr*)&sun, sizeof(sun)) < 0) {
        PLOGE("daemon bind");
        goto err;
    }

    chmod(REQUESTOR_DAEMON_PATH, 0755);
    chmod(sun.sun_path, 0777);

    umask(previous_umask);

    if (listen(fd, 10) < 0) {
        PLOGE("daemon listen");
        goto err;
    }

    int client;
    while ((client = accept(fd, NULL, NULL)) > 0) {
        if (fork_zero_fucks() == 0) {
            close(fd);
            return daemon_accept(client);
        }
        else {
            close(client);
        }
    }

    LOGE("daemon exiting");
err:
    close(fd);
    return -1;
}

// List of signals which cause process termination
static int quit_signals[] = { SIGALRM, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

static void sighandler(int sig) {
	(void)sig;
    restore_stdin();

    // Assume we'll only be called before death
    // See note before sigaction() in set_stdin_raw()
    //
    // Now, close all standard I/O to cause the pumps
    // to exit so we can continue and retrieve the exit
    // code
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Put back all the default handlers
    struct sigaction act;
    int i;

    memset(&act, '\0', sizeof(act));
    act.sa_handler = SIG_DFL;
    for (i = 0; quit_signals[i]; i++) {
        if (sigaction(quit_signals[i], &act, NULL) < 0) {
            PLOGE("Error removing signal handler");
            continue;
        }
    }
}

/**
 * Setup signal handlers trap signals which should result in program termination
 * so that we can restore the terminal to its normal state and retrieve the 
 * return code.
 */
static void setup_sighandlers(void) {
    struct sigaction act;
    int i;

    // Install the termination handlers
    // Note: we're assuming that none of these signal handlers are already trapped.
    // If they are, we'll need to modify this code to save the previous handler and
    // call it after we restore stdin to its previous state.
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sighandler;
    for (i = 0; quit_signals[i]; i++) {
        if (sigaction(quit_signals[i], &act, NULL) < 0) {
            PLOGE("Error installing signal handler");
            continue;
        }
    }
}

int connect_daemon(int argc, char *argv[], int ppid) {
    int uid = getuid();
    int ptmx = -1;
    char pts_slave[PATH_MAX];

    struct sockaddr_un sun;

    // Open a socket to the daemon
    int socketfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (socketfd < 0) {
        PLOGE("socket");
        exit(-1);
    }
    if (fcntl(socketfd, F_SETFD, FD_CLOEXEC)) {
        PLOGE("fcntl FD_CLOEXEC");
        exit(-1);
    }

    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_LOCAL;
    sprintf(sun.sun_path, "%s/server", REQUESTOR_DAEMON_PATH);

    memset(sun.sun_path, 0, sizeof(sun.sun_path));
    memcpy(sun.sun_path, "\0" "SUPERUSER", strlen("SUPERUSER") + 1);

    if (0 != connect(socketfd, (struct sockaddr*)&sun, sizeof(sun))) {
        PLOGE("connect");
        exit(-1);
    }

    LOGD("connecting client %d", getpid());

    int mount_storage = getenv("MOUNT_EMULATED_STORAGE") != NULL;

    // Determine which one of our streams are attached to a TTY
    int atty = 0;

    // Send TTYs directly (instead of proxying with a PTY) if
    // the SUPERUSER_SEND_TTY environment variable is set.
    if (getenv("SUPERUSER_SEND_TTY") == NULL) {
        if (isatty(STDIN_FILENO))  atty |= ATTY_IN;
        if (isatty(STDOUT_FILENO)) atty |= ATTY_OUT;
        if (isatty(STDERR_FILENO)) atty |= ATTY_ERR;
    }

    if (atty) {
        // We need a PTY. Get one.
        ptmx = pts_open(pts_slave, sizeof(pts_slave));
        if (ptmx < 0) {
            PLOGE("pts_open");
            exit(-1);
        }
    } else {
        pts_slave[0] = '\0';
    }

    // Send some info to the daemon, starting with our PID
    write_int(socketfd, getpid());
    // Send the slave path to the daemon
    // (This is "" if we're not using PTYs)
    write_string(socketfd, pts_slave);
    // User ID
    write_int(socketfd, uid);
    // Parent PID
    write_int(socketfd, ppid);
    write_int(socketfd, mount_storage);

    // Send stdin
    if (atty & ATTY_IN) {
        // Using PTY
        send_fd(socketfd, -1);
    } else {
        send_fd(socketfd, STDIN_FILENO);
    }

    // Send stdout
    if (atty & ATTY_OUT) {
        // Forward SIGWINCH
        watch_sigwinch_async(STDOUT_FILENO, ptmx);

        // Using PTY
        send_fd(socketfd, -1);
    } else {
        send_fd(socketfd, STDOUT_FILENO);
    }

    // Send stderr
    if (atty & ATTY_ERR) {
        // Using PTY
        send_fd(socketfd, -1);
    } else {
        send_fd(socketfd, STDERR_FILENO);
    }

    // Number of command line arguments
    write_int(socketfd, mount_storage ? argc - 1 : argc);

    // Command line arguments
    int i;
    for (i = 0; i < argc; i++) {
        if (i == 1 && mount_storage) {
            continue;
        }
        write_string(socketfd, argv[i]);
    }

    // Wait for acknowledgement from daemon
    read_int(socketfd);

    if (atty & ATTY_IN) {
        setup_sighandlers();
        pump_stdin_async(ptmx);
    }
    if (atty & ATTY_OUT) {
        pump_stdout_blocking(ptmx);
    }

    // Get the exit code
    int code = read_int(socketfd);
    close(socketfd);
    LOGD("client exited %d", code);

    return code;
}
