/* list_manager.c - Hide list management
 */

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "magiskhide.h"

int add_list(char *proc) {
	if (!hideEnabled) {
		free(proc);
		return HIDE_NOT_ENABLED;
	}

	char *line;
	struct vector *new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		return DAEMON_ERROR;
	vec_init(new_list);

	vec_for_each(hide_list, line) {
		// They should be unique
		if (strcmp(line, proc) == 0) {
			free(proc);
			vec_destroy(new_list);
			free(new_list);
			return HIDE_ITEM_EXIST;
		}
		vec_push_back(new_list, line);
	}

	vec_push_back(new_list, proc);
	LOGI("hide_list add: [%s]\n", proc);
	ps_filter_proc_name(proc, kill_proc);

	// Critical region
	pthread_mutex_lock(&hide_lock);
	vec_destroy(hide_list);
	free(hide_list);
	hide_list = new_list;
	pthread_mutex_unlock(&hide_lock);

	pthread_mutex_lock(&file_lock);
	if (vector_to_file(HIDELIST, hide_list)) {
		pthread_mutex_unlock(&file_lock);
		return DAEMON_ERROR;
	}
	pthread_mutex_unlock(&file_lock);
	return DAEMON_SUCCESS;
}

int rm_list(char *proc) {
	if (!hideEnabled) {
		free(proc);
		return HIDE_NOT_ENABLED;
	}

	daemon_response ret = DAEMON_ERROR;
	char *line;
	int do_rm = 0;
	struct vector *new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		goto error;
	vec_init(new_list);

	vec_for_each(hide_list, line) {
		if (strcmp(line, proc) == 0) {
			free(proc);
			proc = line;
			do_rm = 1;
			continue;
		}
		vec_push_back(new_list, line);
	}

	if (do_rm) {
		LOGI("hide_list rm: [%s]\n", proc);
		ps_filter_proc_name(proc, kill_proc);
		// Critical region
		pthread_mutex_lock(&hide_lock);
		vec_destroy(hide_list);
		free(hide_list);
		hide_list = new_list;
		pthread_mutex_unlock(&hide_lock);

		ret = DAEMON_SUCCESS;
		pthread_mutex_lock(&file_lock);
		if (vector_to_file(HIDELIST, hide_list))
			ret = DAEMON_ERROR;
		pthread_mutex_unlock(&file_lock);
	} else {
		ret = HIDE_ITEM_NOT_EXIST;
		vec_destroy(new_list);
		free(new_list);
	}

error:
	free(proc);
	return ret;
}

int init_list() {
	LOGD("hide_list: initialize...\n");
	if ((hide_list = xmalloc(sizeof(*hide_list))) == NULL)
		return 1;
	vec_init(hide_list);

	// Might error if file doesn't exist, no need to report
	file_to_vector(HIDELIST, hide_list);

	char *line;
	vec_for_each(hide_list, line) {
		LOGI("hide_list: [%s]\n", line);
		ps_filter_proc_name(line, kill_proc);
	}
	return 0;
}

int destroy_list() {
	char *line;
	vec_for_each(hide_list, line) {
		ps_filter_proc_name(line, kill_proc);
	}
	vec_deep_destroy(hide_list);
	free(hide_list);
	hide_list = NULL;
	return 0;
}

void add_hide_list(int client) {
	err_handler = do_nothing;
	char *proc = read_string(client);
	// ack
	write_int(client, add_list(proc));
	close(client);
}

void rm_hide_list(int client) {
	err_handler = do_nothing;
	char *proc = read_string(client);
	// ack
	write_int(client, rm_list(proc));
	close(client);
}
