#include "magiskhide.h"

static int isMocked = 0;

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

void read_namespace(const int pid, char* target, const size_t size) {
	char path[32];
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);
	ssize_t len = readlink(path, target, size);
	target[len] = '\0';
}

void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		fprintf(logfile, "MagiskHide: Unmounted (%s)\n", mountpoint);
	else
		fprintf(logfile, "MagiskHide: Unmount Failed (%s)\n", mountpoint);
}

void run_as_daemon() {
	switch(fork()) {
		case -1:
			exit(-1);
		case 0:
			if (setsid() < 0)
				exit(-1);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			logfile = fopen(LOGFILE, "a+");
			setbuf(logfile, NULL);
			break;
		default:
			exit(0); 
	}
}

void manage_selinux() {
	if (isMocked) return;
	char val[1];
	int fd = open(ENFORCE_FILE, O_RDONLY);
	if (fd < 0)
		return;
	if (read(fd, val, 1) < 1)
		return;
	close(fd);
	// Permissive
	if (val[0] == '0') {
		fprintf(logfile, "MagiskHide: Permissive detected, hide the state\n");

		chmod(ENFORCE_FILE, 0640);
		chmod(POLICY_FILE, 0440);
		isMocked = 1;
	}
}
