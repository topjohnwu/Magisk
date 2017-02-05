#include "magiskhide.h"

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
	char *argv[] = { SEPOLICY_INJECT, "--live", "permissive *", NULL };
	char str[20];
	int fd, ret;
	fd = open(ENFORCE_FILE, O_RDONLY);
	if (fd < 0)
		return;
	ret = read(fd, str, 20);
	close(fd);
	if (ret < 1)
		return;
	// Permissive
	if (str[0] == '0') {
		fprintf(logfile, "MagiskHide: Permissive detected, switching to pseudo enforced\n");
		fd = open(ENFORCE_FILE, O_RDWR);
		if (fd < 0)
			return;
		ret = write(fd, "1", 1);
		close(fd);
		if (ret < 1)
			return;
		switch(fork()) {
			case -1:
				return;
			case 0:
				execvp(argv[0], argv);
			default:
				return;
		}
	}
}
