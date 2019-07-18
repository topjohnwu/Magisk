/* misc.cpp - Store all functions that are unable to be catagorized clearly
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <syscall.h>
#include <random>

#include <logging.h>
#include <utils.h>

unsigned get_shell_uid() {
	struct passwd* ppwd = getpwnam("shell");
	if (nullptr == ppwd)
		return 2000;

	return ppwd->pw_uid;
}

int fork_dont_care() {
	int pid = xfork();
	if (pid) {
		waitpid(pid, nullptr, 0);
		return pid;
	} else if ((pid = xfork())) {
		exit(0);
	}
	return 0;
}

int fork_no_zombie() {
	int pid = xfork();
	if (pid)
		return pid;
	// Unblock all signals
	sigset_t block_set;
	sigfillset(&block_set);
	pthread_sigmask(SIG_UNBLOCK, &block_set, nullptr);
	prctl(PR_SET_PDEATHSIG, SIGTERM);
	if (getppid() == 1)
		exit(1);
	return 0;
}

constexpr char ALPHANUM[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static bool seeded = false;
static std::mt19937 gen;
static std::uniform_int_distribution<int> dist(0, sizeof(ALPHANUM) - 2);
void gen_rand_str(char *buf, int len, bool varlen) {
	if (!seeded) {
		if (access("/dev/urandom", F_OK) != 0)
			mknod("/dev/urandom", 0600 | S_IFCHR, makedev(1, 9));
		int fd = xopen("/dev/urandom", O_RDONLY | O_CLOEXEC);
		unsigned seed;
		xxread(fd, &seed, sizeof(seed));
		gen.seed(seed);
		close(fd);
		seeded = true;
	}
	if (varlen) {
		std::uniform_int_distribution<int> len_dist(len / 2, len);
		len = len_dist(gen);
	}
	for (int i = 0; i < len - 1; ++i)
		buf[i] = ALPHANUM[dist(gen)];
	buf[len - 1] = '\0';
}

int strend(const char *s1, const char *s2) {
	size_t l1 = strlen(s1);
	size_t l2 = strlen(s2);
	return strcmp(s1 + l1 - l2, s2);
}

int exec_command(exec_t &exec) {
	int pipefd[2] = {-1, -1}, outfd = -1;

	if (exec.fd == -1) {
		if (xpipe2(pipefd, O_CLOEXEC) == -1)
			return -1;
		outfd = pipefd[1];
	} else if (exec.fd >= 0) {
		outfd = exec.fd;
	}

	int pid = exec.fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	} else if (pid) {
		if (exec.fd == -1) {
			exec.fd = pipefd[0];
			close(pipefd[1]);
		}
		return pid;
	}

	if (outfd >= 0) {
		xdup2(outfd, STDOUT_FILENO);
		if (exec.err)
			xdup2(outfd, STDERR_FILENO);
		close(outfd);
	}

	// Call the pre-exec callback
	if (exec.pre_exec)
		exec.pre_exec();

	execve(exec.argv[0], (char **) exec.argv, environ);
	PLOGE("execve %s", exec.argv[0]);
	exit(-1);
}

int exec_command_sync(exec_t &exec) {
	int pid, status;
	pid = exec_command(exec);
	if (pid < 0)
		return -1;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int new_daemon_thread(void *(*start_routine) (void *), void *arg, const pthread_attr_t *attr) {
	pthread_t thread;
	int ret = xpthread_create(&thread, attr, start_routine, arg);
	if (ret == 0)
		pthread_detach(thread);
	return ret;
}

static void *proxy_routine(void *fp) {
	auto fn = reinterpret_cast<std::function<void()>*>(fp);
	(*fn)();
	delete fn;
	return nullptr;
}

int new_daemon_thread(std::function<void()> &&fn) {
	return new_daemon_thread(proxy_routine, new std::function<void()>(std::move(fn)));
}

static char *argv0;
static size_t name_len;
void init_argv0(int argc, char **argv) {
	argv0 = argv[0];
	name_len = (argv[argc - 1] - argv[0]) + strlen(argv[argc - 1]) + 1;
}

void set_nice_name(const char *name) {
	memset(argv0, 0, name_len);
	strlcpy(argv0, name, name_len);
	prctl(PR_SET_NAME, name);
}

bool ends_with(const std::string_view &s1, const std::string_view &s2) {
	unsigned l1 = s1.length();
	unsigned l2 = s2.length();
	return l1 < l2 ? false : s1.compare(l1 - l2, l2, s2) == 0;
}

char *rtrim(char *str) {
	int len = strlen(str);
	while (len > 0 && str[len - 1] == ' ')
		--len;
	str[len] = '\0';
	return str;
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
int parse_int(const char *s) {
	int val = 0;
	char c;
	while ((c = *(s++))) {
		if (c > '9' || c < '0')
			return -1;
		val = val * 10 + c - '0';
	}
	return val;
}
