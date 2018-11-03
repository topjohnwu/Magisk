#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"
#include "logging.h"

/*
   fd == NULL -> Ignore output
  *fd < 0     -> Open pipe and set *fd to the read end
  *fd >= 0    -> STDOUT (or STDERR) will be redirected to *fd
  *setenv     -> A callback function which sets up a vector of environment variables
*/
int exec_array(int err, int *fd, void (*setenv)(struct vector *), char *const *argv) {
	int pipefd[2], writeEnd = -1;

	if (fd) {
		if (*fd < 0) {
			if (xpipe2(pipefd, O_CLOEXEC) == -1)
				return -1;
			writeEnd = pipefd[1];
		} else {
			writeEnd = *fd;
		}
	}

	// Setup environment
	char **envp;
	struct vector env;
	vec_init(&env);
	if (setenv) {
		setenv(&env);
		envp = (char **) vec_entry(&env);
	} else {
		extern char **environ;
		envp = environ;
	}

	int pid = xfork();
	if (pid != 0) {
		if (fd && *fd < 0) {
			// Give the read end and close write end
			*fd = pipefd[0];
			close(pipefd[1]);
		}
		vec_deep_destroy(&env);
		return pid;
	}

	if (fd) {
		xdup2(writeEnd, STDOUT_FILENO);
		if (err)
			xdup2(writeEnd, STDERR_FILENO);
	}

	environ = envp;
	execvp(argv[0], argv);
	PLOGE("execvp %s", argv[0]);
	return -1;
}

static int v_exec_command(int err, int *fd, void (*setenv)(struct vector*), const char *argv0, va_list argv) {
	// Collect va_list into vector
	struct vector args;
	vec_init(&args);
	vec_push_back(&args, strdup(argv0));
	for (void *arg = va_arg(argv, void*); arg; arg = va_arg(argv, void*))
		vec_push_back(&args, strdup(arg));
	vec_push_back(&args, NULL);
	int pid = exec_array(err, fd, setenv, (char **) vec_entry(&args));
	vec_deep_destroy(&args);
	return pid;
}

int exec_command_sync(char *const argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid, status;
	pid = v_exec_command(0, NULL, NULL, argv0, argv);
	va_end(argv);
	if (pid < 0)
		return pid;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int exec_command(int err, int *fd, void (*setenv)(struct vector*), const char *argv0, ...) {
	va_list argv;
	va_start(argv, argv0);
	int pid = v_exec_command(err, fd, setenv, argv0, argv);
	va_end(argv);
	return pid;
}

/* All the string should be freed manually!! */
int file_to_vector(const char* filename, struct vector *v) {
	if (access(filename, R_OK) != 0)
		return 1;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE *fp = xfopen(filename, "r");
	if (fp == NULL)
		return 1;

	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove end newline
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		vec_push_back(v, line);
		line = NULL;
	}
	fclose(fp);
	return 0;
}

int vector_to_file(const char *filename, struct vector *v) {
	FILE *fp = xfopen(filename, "w");
	if (fp == NULL)
		return 1;
	char *line;
	vec_for_each(v, line) {
		fprintf(fp, "%s\n", line);
	}
	fclose(fp);
	return 0;
}