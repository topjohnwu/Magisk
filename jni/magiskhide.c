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

#define HIDE_LIST "/magisk/.core/magiskhide/hidelist"
#define MOUNT_LIST "/dev/mountlist"

int hide_size = 0, mount_size = 0;
char **hide_list, **mount_list;
time_t last_update = 0;

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

	int i;
	for(i = mount_size - 1; i >= 0; --i) {
		res = umount2(mount_list[i], MNT_DETACH);
		if (res != -1) printf("Unmounted: %s\n", mount_list[i]);
		else printf("Failed: %s\n", mount_list[i]);
	}
	res = umount2("/magisk", MNT_DETACH);
	if (res != -1) printf("Unmounted: %s\n", "/magisk");
	else printf("Failed: %s\n", "/magisk");

	return 0;
}

char** file_to_str_arr(const int fd, int *size) {
	int allocated = 16;
	char *buffer, *tok, **array;
	
	off_t filesize = lseek(fd, 0, SEEK_END);
	buffer = malloc(sizeof(char) * filesize);
	lseek(fd, 0, SEEK_SET);
	read(fd, buffer, filesize);

	array = (char **) malloc(sizeof(char*) * allocated);

	*size = 0;
	tok = strtok(buffer, "\r\n");
    while (tok != NULL) {
    	if (*size >= allocated) {
    		// Double our allocation and re-allocate
    		allocated = allocated * 2;
    		array = (char **) realloc(array, sizeof(char*) * allocated);
    	}
    	if (strlen(tok)) {
    		array[*size] = malloc(strlen(tok));
    		strcpy(array[*size], tok);
    		++(*size);
    	}
        tok = strtok(NULL, "\r\n");
    }
	free(buffer);
	return array;
}

int load_hide_list(const int fd) {
	int i;
	struct stat file_stat;

	fstat(fd, &file_stat);
	if (file_stat.st_mtime == last_update) {
		return 0;
	}

	// Free memory
	for(i = 0; i < hide_size; ++i)
		free(hide_list[i]);
	free(hide_list);

	hide_list = file_to_str_arr(fd, &hide_size);

    fstat(fd, &file_stat);
	last_update = file_stat.st_mtime;

	printf("Get package name from config:\n");
	for(i = 0; i < hide_size; i++)
		printf("%s\n", hide_list[i]);
	printf("\n");
}

int main(int argc, char **argv, char **envp) {
	int i;

	int hide_fd = open(HIDE_LIST, O_RDONLY);
	if (hide_fd == -1){
		printf("Error opening hide list\n");
		exit(1);
	}

	int mount_fd = open(MOUNT_LIST, O_RDONLY);
	if (mount_fd == -1){
		printf("Error opening mount list\n");
		exit(1);
	}

	mount_list = file_to_str_arr(mount_fd, &mount_size);
	close(mount_fd);

	char buffer[512];
	FILE *p = popen("while true;do logcat -b events -v raw -s am_proc_start;sleep 1;done", "r");
	while(!feof(p)) {

		load_hide_list(hide_fd);

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
	close(hide_fd);

	// Free memory
	for(i = 0; i < hide_size; ++i)
		free(hide_list[i]);
	free(hide_list);

	for(i = 0; i < mount_size; ++i)
		free(mount_list[i]);
	free(mount_list);

	return 0;
}
