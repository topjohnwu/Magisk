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
	struct vector *new_list, *temp = hide_list;
	new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		return HIDE_ERROR;
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
	hide_list = new_list;
	pthread_mutex_unlock(&hide_lock);

	// Free old list
	vec_destroy(temp);
	free(temp);
	if (vector_to_file(HIDELIST, hide_list))
		return HIDE_ERROR;
	return HIDE_SUCCESS;
}

int rm_list(char *proc) {
	if (!hideEnabled) {
		free(proc);
		return HIDE_NOT_ENABLED;
	}

	hide_ret ret = HIDE_ERROR;
	char *line;
	struct vector *new_list, *temp;
	temp = new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		goto error;
	vec_init(new_list);

	vec_for_each(hide_list, line) {
		if (strcmp(line, proc) == 0) {
			free(proc);
			proc = line;
			temp = hide_list;
			continue;
		}
		vec_push_back(new_list, line);
	}

	if (temp == hide_list) {
		LOGI("hide_list rm: [%s]\n", proc);
		ps_filter_proc_name(proc, kill_proc);
		// Critical region
		pthread_mutex_lock(&hide_lock);
		hide_list = new_list;
		pthread_mutex_unlock(&hide_lock);
		ret = HIDE_SUCCESS;
		if (vector_to_file(HIDELIST, hide_list))
			ret = HIDE_ERROR;
	} else {
		ret = HIDE_ITEM_NOT_EXIST;
	}

error:
	free(proc);
	vec_destroy(temp);
	free(temp);
	return ret;
}

int init_list() {
	LOGD("hide_list: initialize...\n");
	if ((hide_list = xmalloc(sizeof(*hide_list))) == NULL)
		return 1;
	vec_init(hide_list);

	// Might return 1 if first time
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
	char *proc = read_string(client);
	// ack
	write_int(client, add_list(proc));
	close(client);
}

void rm_hide_list(int client) {
	char *proc = read_string(client);
	// ack
	write_int(client, rm_list(proc));
	close(client);
}
