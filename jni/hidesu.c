#define _GNU_SOURCE
#include <sched.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/syscall.h>

int main(int argc, char **argv) {
	if(argc != 2) exit(5);
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) exit(2);
//TODO: Fix non arm platforms
#define SYS_setns 375
	int res = syscall(SYS_setns, fd, 0);
	if(res == -1) exit(3);

	//XXX: What to mount to /sbin...?
	res = mount("/system", "/sbin", "bind", MS_BIND, "");
	if(res == -1) exit(4);
	exit(0);
}
