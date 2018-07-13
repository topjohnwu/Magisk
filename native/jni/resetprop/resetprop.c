/* resetprop.cpp - Manipulate any system props
 *
 * Copyright 2016 nkk71     <nkk71x@gmail.com>
 * Copyright 2016 topjohnwu <topjohnwu@gmail.com>
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
#include "private/_system_properties.h"
#include "private/system_properties.h"

#include "magisk.h"
#include "resetprop.h"
#include "_resetprop.h"
#include "vector.h"
#include "utils.h"

int prop_verbose = 0;

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

// The callback passes to __system_property_read_callback, actually runs the callback in read_cb
static void callback_wrapper(void *read_cb, const char *name, const char *value, uint32_t serial) {
	((struct read_cb_t *) read_cb)->func(name, value, ((struct read_cb_t *) read_cb)->cookie);
}

/* **********************************
 * Callback functions for read_cb_t
 * **********************************/

void collect_props(const char *name, const char *value, void *prop_list) {
	struct prop_t *p = (struct prop_t *) xmalloc(sizeof(*p));
	p->name = strdup(name);
	strcpy(p->value, value);
	vec_push_back(prop_list, p);
}

static void collect_unique_props(const char *name, const char *value, void *prop_list) {
	struct vector *v = prop_list;
	struct prop_t *p;
	bool uniq = true;
	vec_for_each(v, p) {
		if (strcmp(name, p->name) == 0) {
			uniq = 0;
			break;
		}
	}
	if (uniq)
		collect_props(name, value, prop_list);
}

static void store_prop_value(const char *name, const char *value, void *dst) {
	strcpy(dst, value);
}

static void prop_foreach_cb(const prop_info* pi, void* read_cb) {
	__system_property_read_callback(pi, callback_wrapper, read_cb);
}

// Comparision function used to sort prop vectors
static int prop_cmp(const void *p1, const void *p2) {
	return strcmp(((struct prop_t *) p1)->name, ((struct prop_t *) p2)->name);
}

static int init_resetprop() {
	if (__system_properties_init()) {
		PRINT_E("resetprop: Initialize error\n");
		return -1;
	}
	return 0;
}

static void print_props(int persist) {
	struct prop_t *p;
	struct vector prop_list;
	vec_init(&prop_list);
	getprop_all(collect_props, &prop_list);
	if (persist) {
		struct read_cb_t read_cb = {
			.func = collect_unique_props,
			.cookie = &prop_list
		};
		persist_getprop_all(&read_cb);
	}
	vec_sort(&prop_list, prop_cmp);
	vec_for_each(&prop_list, p) {
		printf("[%s]: [%s]\n", p->name, p->value);
		free(p->name);
		free(p);
	}
	vec_destroy(&prop_list);
}

/* **************************************************
 * Implementations of functions in resetprop.h (APIs)
 * **************************************************/

int prop_exist(const char *name) {
	if (init_resetprop()) return 0;
	return __system_property_find(name) != NULL;
}

char *getprop(const char *name) {
	return getprop2(name, 0);
}

// Get prop by name, return string (should free manually!)
char *getprop2(const char *name, int persist) {
	if (check_legal_property_name(name))
		return NULL;
	if (init_resetprop()) return NULL;
	const prop_info *pi = __system_property_find(name);
	if (pi == NULL) {
		if (persist && strncmp(name, "persist.", 8) == 0) {
			char *value = persist_getprop(name);
			if (value)
				return value;
		}
		PRINT_D("resetprop: prop [%s] does not exist\n", name);
		return NULL;
	} else {
		char value[PROP_VALUE_MAX];
		struct read_cb_t read_cb = {
			.func = store_prop_value,
			.cookie = value
		};
		__system_property_read_callback(pi, callback_wrapper, &read_cb);
		PRINT_D("resetprop: getprop [%s]: [%s]\n", name, value);
		return strdup(value);
	}
}

void getprop_all(void (*callback)(const char *, const char *, void *), void *cookie) {
	if (init_resetprop()) return;
	struct read_cb_t read_cb = {
		.func = callback,
		.cookie = cookie
	};
	__system_property_foreach(prop_foreach_cb, &read_cb);
}

int setprop(const char *name, const char *value) {
	return setprop2(name, value, 1);
}

int setprop2(const char *name, const char *value, const int trigger) {
	if (check_legal_property_name(name))
		return 1;
	if (init_resetprop()) return -1;
	int ret;

	prop_info *pi = (prop_info*) __system_property_find(name);
	if (pi != NULL) {
		if (trigger) {
			if (strncmp(name, "ro.", 3) == 0) deleteprop(name);
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_update(pi, value, strlen(value));
		}
	} else {
		PRINT_D("resetprop: New prop [%s]\n", name);
		if (trigger) {
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_add(name, strlen(name), value, strlen(value));
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

int deleteprop2(const char *name, int persist) {
	if (check_legal_property_name(name))
		return 1;
	if (init_resetprop()) return -1;
	char path[PATH_MAX];
	path[0] = '\0';
	PRINT_D("resetprop: deleteprop [%s]\n", name);
	if (persist && strncmp(name, "persist.", 8) == 0)
		persist = persist_deleteprop(name);
	return __system_property_del(name) && !(persist && strncmp(name, "persist.", 8) == 0);
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
		// Ignore invalid formats
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
				prop_verbose = 1;
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
		print_props(persist);
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
