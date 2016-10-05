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
#include <sys/mount.h> 

#define HIDE_LIST "/magisk/.core/hidelist"

//WARNING: Calling this will change our current namespace
//We don't care because we don't want to run from here anyway
int hideMagisk(int pid) {
	char *path = NULL;
	asprintf(&path, "/proc/%d/ns/mnt", pid);
	int fd = open(path, O_RDONLY);
	if(fd == -1) return 2;
//TODO: Fix non arm platforms
#define SYS_setns 375
	int res = syscall(SYS_setns, fd, 0);
	if(res == -1) return 3;

	res = umount2("/magisk", MNT_DETACH);
	if(res == -1) return 4;
	res = mount("/magisk/.core/mirror/system", "/system", "bind", MS_BIND, "");
	if(res == -1) return 4;
	return 0;
}

int main(int argc, char **argv, char **envp) {
	// Read config file
	int allocated = 16, line = 0, i;
	char buffer[512];
	
	char **list = (char **) malloc(sizeof(char*) * allocated);
	FILE *fp = fopen(HIDE_LIST, "r");
	if (fp == NULL){
		fprintf(stderr, "Error opening hide list\n");
		exit(1);
	}
	while (1) {
		if (fgets(buffer, sizeof(buffer), fp) == NULL) {
			--line;
			break;
		}
		if (line >= allocated) {
			// Double our allocation and re-allocate
			allocated = allocated * 2;
			list = (char **) realloc(list, sizeof(char*) * allocated);
		}
		list[line] = malloc(strlen(buffer));
		strcpy(list[line], buffer);
		int j;
		// Remove endline
		for (j = strlen(list[line]) - 1; j >= 0 && (list[line][j] == '\n' || list[line][j] == '\r'); j--)
			;
		list[line][j + 1] = '\0';
		++line;
	}
	fclose(fp);

	printf("Get package name from config:\n");
	for(i = 0; i <= line; i++)
		printf("%s\n", list[i]);
	printf("\n");


	FILE *p = popen("while true;do logcat -b events -v raw -s am_proc_start;sleep 1;done", "r");
	while(!feof(p)) {
		//Format of am_proc_start is (as of Android 5.1 and 6.0)
		//UserID, pid, unix uid, processName, hostingType, hostingName
		fgets(buffer, sizeof(buffer), p);

		{
			char *pos = buffer;
			while(1) {
				pos = strchr(pos, ',');
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
			continue;
		}

		for (i = 0; i <= line; ++i) {
			if(strstr(processName, list[i]) != NULL) {
				printf("Disabling for process = %s, PID = %d, UID = %d\n", processName, pid, uid);
				hideMagisk(pid);
			}
		}
	}

	pclose(p);
	for (; line >= 0; line--)
		free(list[line]);
	free(list);

	return 0;
}
