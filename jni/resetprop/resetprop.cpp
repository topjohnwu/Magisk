/* resetprop.cpp - Manipulate any system props
 *
 * Copyright 2016 nkk71     <nkk71x@gmail.com>
 * Copyright 2016 topjohnwu <topjohnwu@gmail.com>
 *
 * Info:
 *
 * all changes are in
 *
 * bionic/libc/bionic/system_properties.cpp
 *
 * Functions that need to be patched/added in system_properties.cpp
 *
 * int __system_properties_init2()
 *     on android 7, first tear down the everything then let it initialize again:
 *         if (initialized) {
 *             //list_foreach(contexts, [](context_node* l) { l->reset_access(); });
 *             //return 0;
 *             free_and_unmap_contexts();
 *             initialized = false;
 *         }
 *
 *
 * static prop_area* map_prop_area(const char* filename, bool is_legacy)
 *     we dont want this read only so change: 'O_RDONLY' to 'O_RDWR'
 *
 * static prop_area* map_fd_ro(const int fd)
 *     we dont want this read only so change: 'PROT_READ' to 'PROT_READ | PROT_WRITE'
 *
 *
 * Copy the code of prop_info *prop_area::find_property, and modify to delete props
 * const prop_info *prop_area::find_property_and_del(prop_bt *const trie, const char *name)
 * {
 *    ...
 *    ...  Do not alloc a new prop_bt here, remove all code involve alloc_if_needed
 *    ...
 *
 *     if (prop_offset != 0) {
 *         atomic_store_explicit(&current->prop, 0, memory_order_release); // Add this line to nullify the prop entry
 *         return to_prop_info(&current->prop);
 *     } else {
 *
 *    ....
 * }
 *
 *
 * by patching just those functions directly, all other functions should be ok
 * as is.
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "_system_properties.h"
#include "system_properties.h"

#include "magisk.h"
#include "resetprop.h"
extern "C" {
#include "vector.h"
}

#define PRINT_D(...)  { LOGD(__VA_ARGS__); if (verbose) fprintf(stderr, __VA_ARGS__); }
#define PRINT_E(...)  { LOGE(__VA_ARGS__); fprintf(stderr, __VA_ARGS__); }
#define PERSISTENT_PROPERTY_DIR  "/data/property"

static int verbose = 0;

static int check_legal_property_name(const char *name) {
	int namelen = strlen(name);

	if (namelen < 1) goto illegal;
	if (name[0] == '.') goto illegal;
	if (name[namelen - 1] == '.') goto illegal;

	/* Only allow alphanumeric, plus '.', '-', '@', ':', or '_' */
	/* Don't allow ".." to appear in a property name */
	for (size_t i = 0; i < namelen; i++) {
		if (name[i] == '.') {
			// i=0 is guaranteed to never have a dot. See above.
			if (name[i-1] == '.') goto illegal;
			continue;
		}
		if (name[i] == '_' || name[i] == '-' || name[i] == '@' || name[i] == ':') continue;
		if (name[i] >= 'a' && name[i] <= 'z') continue;
		if (name[i] >= 'A' && name[i] <= 'Z') continue;
		if (name[i] >= '0' && name[i] <= '9') continue;
		goto illegal;
	}

	return 0;

illegal:
	PRINT_E("Illegal property name: [%s]\n", name);
	return 1;
}

static int usage(char* arg0) {
	fprintf(stderr,
		"resetprop v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu & nkk71) - System Props Modification Tool\n\n"
		"Usage: %s [flags] [options...]\n"
		"\n"
		"Options:\n"
		"   -h, --help        show this message\n"
		"   (no arguments)    print all properties\n"
		"   NAME              get property\n"
		"   NAME VALUE        set property entry NAME with VALUE\n"
		"   --file FILE       load props from FILE\n"
		"   --delete NAME     delete property\n"
		"\n"
		"Flags:\n"
		"   -v      print verbose output to stderr\n"
		"   -n      set properties without init triggers\n"
		"           only affects setprop\n"
		"   -p      access actual persist storage\n"
		"           only affects getprop and deleteprop\n"
		"\n"

	, arg0);
	return 1;
}

static int init_resetprop() {
	if (__system_properties_init2()) {
		PRINT_E("resetprop: Initialize error\n");
		return -1;
	}
	return 0;
}

int prop_exist(const char *name) {
	if (init_resetprop()) return 0;
	return __system_property_find2(name) != NULL;
}

static void read_prop_info(void* cookie, const char *name, const char *value, uint32_t serial) {
	strcpy((char *) cookie, value);
}


char *getprop(const char *name) {
	return getprop2(name, 0);
}

// Get prop by name, return string (should free manually!)
char *getprop2(const char *name, int persist) {
	if (check_legal_property_name(name))
		return NULL;
	char value[PROP_VALUE_MAX];
	if (init_resetprop()) return NULL;
	const prop_info *pi = __system_property_find2(name);
	if (pi == NULL) {
		if (persist && strncmp(name, "persist.", 8) == 0) {
			// Try to read from file
			char path[PATH_MAX];
			snprintf(path, sizeof(path), PERSISTENT_PROPERTY_DIR "/%s", name);
			int fd = open(path, O_RDONLY | O_CLOEXEC);
			if (fd < 0) goto no_prop;
			PRINT_D("resetprop: read prop from [%s]\n", path);
			size_t len = read(fd, value, sizeof(value));
			value[len] = '\0';  // Null terminate the read value
		} else {
no_prop:
		PRINT_D("resetprop: prop [%s] does not exist\n", name);
		return NULL;
		}
	} else {
		__system_property_read_callback2(pi, read_prop_info, value);
	}
	PRINT_D("resetprop: getprop [%s]: [%s]\n", name, value);
	return strdup(value);
}

struct wrapper {
	void (*func)(const char *, const char *);
};

static void cb_wrapper(void* cookie, const char *name, const char *value, uint32_t serial) {
	((wrapper *) cookie)->func(name, value);
}

static void prop_foreach_cb(const prop_info* pi, void* cookie) {
	__system_property_read_callback2(pi, cb_wrapper, cookie);
}

class property {
public:
	property(const char *n, const char *v) {
		name = strdup(n);
		value = strdup(v);
	}
	~property() {
		free((void *)name);
		free((void *)value);
	}
	const char *name;
	const char *value;
};

vector prop_list;

static int prop_cmp(const void *p1, const void *p2) {
	return strcmp((*((property **) p1))->name, (*((property **) p2))->name);
}

static void print_all_props_cb(const char *name, const char *value) {
	vec_push_back(&prop_list, new property(name, value));
}

static void print_all_props(int persist) {
	void *p;
	vec_init(&prop_list);
	getprop_all(print_all_props_cb);
	if (persist) {
		// Check all persist props in data
		DIR *dir = opendir(PERSISTENT_PROPERTY_DIR);
		struct dirent *entry;
		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 )
				continue;
			int found = 0;
			vec_for_each(&prop_list, p) {
				if (strcmp(((property *) p)->name, entry->d_name) == 0) {
					found = 1;
					break;
				}
			}
			if (!found)
				vec_push_back(&prop_list, new property(entry->d_name, getprop2(entry->d_name, 1)));
		}
	}
	vec_sort(&prop_list, prop_cmp);
	vec_for_each(&prop_list, p) {
		printf("[%s]: [%s]\n", ((property *) p)->name, ((property *) p)->value);
		delete((property *) p);
	}
	vec_destroy(&prop_list);
}

void getprop_all(void (*callback)(const char*, const char*)) {
	if (init_resetprop()) return;
	struct wrapper wrap = {
		.func = callback
	};
	__system_property_foreach2(prop_foreach_cb, &wrap);
}

int setprop(const char *name, const char *value) {
	return setprop2(name, value, 1);
}

int setprop2(const char *name, const char *value, const int trigger) {
	if (check_legal_property_name(name))
		return 1;
	if (init_resetprop()) return -1;
	int ret;

	prop_info *pi = (prop_info*) __system_property_find2(name);
	if (pi != NULL) {
		if (trigger) {
			if (strncmp(name, "ro.", 3) == 0) deleteprop(name);
			ret = __system_property_set2(name, value);
		} else {
			ret = __system_property_update2(pi, value, strlen(value));
		}
	} else {
		PRINT_D("resetprop: New prop [%s]\n", name);
		if (trigger) {
			ret = __system_property_set2(name, value);
		} else {
			ret = __system_property_add2(name, strlen(name), value, strlen(value));
		}
	}

	PRINT_D("resetprop: setprop [%s]: [%s] by %s\n", name, value,
		trigger ? "property_service" : "modifing prop data structure");

	if (ret)
		PRINT_E("resetprop: setprop error\n");

	return ret;
}

int deleteprop(const char *name) {
	return deleteprop2(name, 1);
}

int deleteprop2(const char *name, const int persist) {
	if (check_legal_property_name(name))
		return 1;
	if (init_resetprop()) return -1;
	char path[PATH_MAX];
	path[0] = '\0';
	PRINT_D("resetprop: deleteprop [%s]\n", name);
	if (persist && strncmp(name, "persist.", 8) == 0)
		snprintf(path, sizeof(path), PERSISTENT_PROPERTY_DIR "/%s", name);
	return __system_property_del(name) && unlink(path);
}

int read_prop_file(const char* filename, const int trigger) {
	if (init_resetprop()) return -1;
	PRINT_D("resetprop: Load prop file [%s]\n", filename);
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		PRINT_E("Cannot open [%s]\n", filename);
		return 1;
	}
	char *line = NULL, *pch;
	size_t len;
	ssize_t read;
	int comment = 0, i;
	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove the trailing newline
		if (line[read - 1] == '\n') {
			line[read - 1] = '\0';
			--read;
		}
		comment = 0;
		for (i = 0; i < read; ++i) {
			// Ignore starting spaces
			if (line[i] == ' ') continue;
			else {
				// A line starting with # is ignored
				if (line[i] == '#') comment = 1;
				break;
			}
		}
		if (comment) continue;
		pch = strchr(line, '=');
		// Ignore ivalid formats
		if ( ((pch == NULL) || (i >= (pch - line))) || (pch >= line + read - 1) ) continue;
		// Separate the string
		*pch = '\0';
		setprop2(line + i, pch + 1, trigger);
	}
	free(line);
	fclose(fp);
	return 0;
}

int resetprop_main(int argc, char *argv[]) {
	int trigger = 1, persist = 0;
	char *argv0 = argv[0], *prop;

	--argc;
	++argv;

	// Parse flags and -- options
	while (argc && argv[0][0] == '-') {
		for (int idx = 1; 1; ++idx) {
			switch (argv[0][idx]) {
			case '-':
				if (strcmp(argv[0], "--file") == 0 && argc == 2) {
					return read_prop_file(argv[1], trigger);
				} else if (strcmp(argv[0], "--delete") == 0 && argc == 2) {
					return deleteprop2(argv[1], persist);
				} else if (strcmp(argv[0], "--help") == 0) {
					goto usage;
				}
			case 'v':
				verbose = 1;
				continue;
			case 'p':
				persist = 1;
				continue;
			case 'n':
				trigger = 0;
				continue;
			case '\0':
				break;
			case 'h':
			default:
			usage:
				return usage(argv0);
			}
			break;
		}
		--argc;
		++argv;
	}

	switch (argc) {
	case 0:
		print_all_props(persist);
		return 0;
	case 1:
		prop = getprop2(argv[0], persist);
		if (prop == NULL) return 1;
		printf("%s\n", prop);
		free(prop);
		return 0;
	case 2:
		return setprop2(argv[0], argv[1], trigger);
	default:
		usage(argv0);
		return 1;
	}
}
