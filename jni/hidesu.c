typedef unsigned short int sa_family_t;
//Linux includes
#define _LINUX_TIME_H
#define _GNU_SOURCE
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <linux/netlink.h>
#include <linux/fs.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

//WARNING: Calling this will change our current namespace
//We don't care because we don't want to run from here anyway
int disableSu(int pid) {
	char *path = NULL;
	asprintf(&path, "/proc/%d/ns/mnt", pid);
	int fd = open(path, O_RDONLY);
	if(fd == -1) exit(2);
//TODO: Fix non arm platforms
#define SYS_setns 375
	int res = syscall(SYS_setns, fd, 0);
	if(res == -1) exit(3);

	//XXX: What to mount to /sbin...?
	res = mount("/system", "/sbin", "bind", MS_BIND, "");
	if(res == -1) exit(4);
	return 0;
}

int main(int argc, char **argv, char **envp) {
	system("logcat -b events -c");
	FILE *p = popen("logcat -b events -v raw -s am_proc_start", "r");
	while(!feof(p)) {
		//Format of am_proc_start is (as of Android 5.1 and 6.0)
		//UserID, pid, unix uid, processName, hostingType, hostingName
		char buffer[512];
		int size = fgets(buffer, sizeof(buffer), p);

		{
			char *pos = buffer;
			while(1) {
				pos = index(pos, ',');
				if(pos == NULL)
					break;
				pos[0] = ' ';
			}
		}

		int user, pid, uid;
		char processName[256], hostingType[16], hostingName[256];
		int ret = sscanf(buffer, "[%d %d %d %256s %16s %256s]",
				&user, &pid, &uid,
				processName, hostingType, hostingName);


		if(ret != 6) {
			printf("sscanf returned %d on '%s'\n", ret, buffer);
			exit(1);
		}
#define GMS_PROC "com.google.android.gms.unstable"
		if(strcmp(processName, GMS_PROC) == 0) {
			printf("Disabling for PID = %d, UID = %d\n", pid, uid);
			disableSu(pid);
		}
	}

	return 0;
}
