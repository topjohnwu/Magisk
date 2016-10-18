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
#include <sys/syscall.h>
#include <asm/unistd.h>
#include <sys/mount.h> 

char **file_to_str_arr(FILE *fp, int *size) {
	int allocated = 16;
	char *line = NULL, **array;
	size_t len = 0;
	ssize_t read;

	array = (char **) malloc(sizeof(char*) * allocated);

	*size = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (*size >= allocated) {
			// Double our allocation and re-allocate
			allocated *= 2;
			array = (char **) realloc(array, sizeof(char*) * allocated);
		}
		// Remove end newline
		if (line[read - 1] == '\n') {
			line[read - 1] = '\0';
		}
		array[*size] = line;
		line = NULL;
		++(*size);
	}
	return array;
}

//WARNING: Calling this will change our current namespace
//We don't care because we don't want to run from here anyway
int hideMagisk(int pid) {
	char *path = NULL;
	asprintf(&path, "/proc/%d/ns/mnt", pid);
	int fd = open(path, O_RDONLY);
	if(fd == -1) return 2;
	int res = syscall(SYS_setns, fd, 0);
	if(res == -1) return 3;

	free(path);
	path = NULL;
	asprintf(&path, "/proc/%d/mounts", pid);
	FILE *mount_fp = fopen(path, "r");
	if (mount_fp == NULL) {
		fprintf(stderr, "Error opening mount list!\n");
		return 1;
	}
	free(path);

	int mount_size;
	char **mount_list = file_to_str_arr(mount_fp, &mount_size), mountpoint[256], *sbstr;
	fclose(mount_fp);

	int i, unmount = 0;
	for(i = mount_size - 1; i >= 0; --i) {
		if((strstr(mount_list[i], "/dev/block/loop") != NULL)) {
			sscanf(mount_list[i], "%256s %256s", mountpoint, mountpoint);
			if (strstr(mountpoint, "/.core/dummy") != NULL) 
				unmount = 0;
			else
				unmount = 1;
		} else if ((sbstr = strstr(mount_list[i], "/.core/dummy")) != NULL) {
			sscanf(sbstr, "/.core/dummy%256s", mountpoint);
			unmount = 1;
		}
		if(unmount) {
			unmount = 0;
			res = umount2(mountpoint, MNT_DETACH);
			if (res != -1) printf("Unmounted: %s\n", mountpoint);
			else printf("Failed: %s\n", mountpoint);
		}
		free(mount_list[i]);
	}
	// Free memory
	free(mount_list);

	return 0;
}

int main(int argc, char **argv, char **envp) {
	if (argc != 2) {
		fprintf(stderr, "%s <process/package name list>\n", argv[0]);
		return 1;
	}
	int i, hide_size;
	char **hide_list;

	FILE *hide_fp = fopen(argv[1], "r");
	if (hide_fp == NULL) {
		fprintf(stderr, "Error opening hide list\n");
		return 1;
	}

	hide_list = file_to_str_arr(hide_fp, &hide_size);
	fclose(hide_fp);

	printf("Get process / package name from config:\n");
	for(i = 0; i < hide_size; i++)
		printf("%s\n", hide_list[i]);
	printf("\n");

	char buffer[512];
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
		for (i = 0; i < hide_size; ++i) {
			if(strstr(processName, hide_list[i]) != NULL) {
				printf("Disabling for process = %s, PID = %d, UID = %d\n", processName, pid, uid);
				hideMagisk(pid);
				break;
			}
		}
	}

	pclose(p);

	// Free memory
	for(i = 0; i < hide_size; ++i)
		free(hide_list[i]);
	free(hide_list);

	return 0;
}
