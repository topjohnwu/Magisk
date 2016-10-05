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
#include <sys/inotify.h>

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

	res = mount("/magisk/.core/mirror/system", "/system", "bind", MS_BIND, "");
	if(res == -1) return 4;
	res = umount2("/magisk", MNT_DETACH);
	if(res == -1) return 4;
	return 0;
}

int loadList(int fd, char ***list, int *line, time_t *last_update) {
	int allocated = 16, i;
	char *buffer, *tok;
	struct stat file_stat;

	fstat(fd, &file_stat);
	if (file_stat.st_mtime == *last_update) {
		return 0;
	}
	off_t filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	buffer = malloc(sizeof(char) * filesize);
	read(fd, buffer, filesize);
	fstat(fd, &file_stat);
	*last_update = file_stat.st_mtime;

	// Free memory
	for (; *line >= 0; --(*line))
		free((*list)[*line]);
	*line = 0;

	*list = (char **) malloc(sizeof(char*) * allocated);

	tok = strtok(buffer, "\r\n");
    while (tok != NULL) {
    	if (*line >= allocated) {
    		// Double our allocation and re-allocate
    		allocated = allocated * 2;
    		*list = (char **) realloc((*list), sizeof(char*) * allocated);
    	}
    	if (strlen(tok)) {
    		(*list)[*line] = malloc(strlen(tok));
    		strcpy((*list)[*line], tok);
    		++(*line);
    	}
        tok = strtok(NULL, "\r\n");
    }

	printf("Get package name from config:\n");
	for(i = 0; i < *line; i++)
		printf("%s\n", (*list)[i]);
	printf("\n");
	free(buffer);
}

int main(int argc, char **argv, char **envp) {
	int line = -1, i;
	char **list;
	time_t last_update = 0;

	int fd = open(HIDE_LIST, O_RDONLY);
	if (fd == -1){
		printf("Error opening file\n");
		exit(1);
	}

	char buffer[512];
	FILE *p = popen("while true;do logcat -b events -v raw -s am_proc_start;sleep 1;done", "r");
	while(!feof(p)) {

		loadList(fd, &list, &line, &last_update);

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
		for (i = 0; i < line; ++i) {
			if(strstr(processName, list[i]) != NULL) {
				printf("Disabling for process = %s, PID = %d, UID = %d\n", processName, pid, uid);
				hideMagisk(pid);
			}
		}
	}

	close(fd);
	pclose(p);
	for (; line >= 0; line--)
		free(list[line]);
	free(list);

	return 0;
}
