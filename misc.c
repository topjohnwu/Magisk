/* misc.c - Miscellaneous stuffs for su
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "magisk.h"
#include "su.h"

void set_identity(unsigned uid) {
	/*
	 * Set effective uid back to root, otherwise setres[ug]id will fail
	 * if uid isn't root.
	 */
	if (seteuid(0)) {
		PLOGE("seteuid (root)");
	}
	if (setresgid(uid, uid, uid)) {
		PLOGE("setresgid (%u)", uid);
	}
	if (setresuid(uid, uid, uid)) {
		PLOGE("setresuid (%u)", uid);
	}
}

char *get_command(const struct su_request *to) {
	if (to->command)
		return to->command;
	if (to->shell)
		return to->shell;
	return DEFAULT_SHELL;
}

int fork_zero_fucks() {
	int pid = fork();
	if (pid) {
		int status;
		waitpid(pid, &status, 0);
		return pid;
	} else {
		if (fork() != 0)
			exit(0);
		return 0;
	}
}
