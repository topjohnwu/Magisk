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
#include "utils.h"
#include "array.h"
#include "flags.h"

bool use_pb = false;
static bool verbose = false;

static bool check_legal_property_name(const char *name) {
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

	return true;

illegal:
	LOGE("Illegal property name: [%s]\n", name);
	return false;
}

[[noreturn]] static void usage(char* arg0) {
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
	exit(1);
}

// Define the way to sort prop_t
template<>
int(*Array<prop_t>::_cmp)(prop_t&, prop_t&) = [](auto a, auto b) -> int {
	return strcmp(a.name, b.name);
};

static void read_props(const prop_info *pi, void *read_cb) {
	__system_property_read_callback(
			pi, [](auto cb, auto name, auto value, auto) -> void
			{
				((read_cb_t *) cb)->exec(name, value);
			}, read_cb);
}

void collect_props(const char *name, const char *value, void *v_plist) {
	Array<prop_t> &prop_list = *static_cast<Array<prop_t> *>(v_plist);
	prop_list.push_back(prop_t(name, value));
}

static void collect_unique_props(const char *name, const char *value, void *v_plist) {
	Array<prop_t> &prop_list = *static_cast<Array<prop_t> *>(v_plist);
	for (auto &prop : prop_list) {
		if (strcmp(name, prop.name) == 0)
			return;
	}
	collect_props(name, value, v_plist);
}

static int init_resetprop() {
	use_pb = use_pb ? true : access(PERSISTENT_PROPERTY_DIR "/persistent_properties", R_OK) == 0;
	if (__system_properties_init()) {
		LOGE("resetprop: Initialize error\n");
		return -1;
	}
	return 0;
}

static void print_props(int persist) {
	auto prop_list = Array<prop_t>();
	getprop_all(collect_props, &prop_list);
	if (persist) {
		read_cb_t read_cb(collect_unique_props, &prop_list);
		persist_getprop_all(&read_cb);
	}
	prop_list.sort();
	for (auto &prop : prop_list)
		printf("[%s]: [%s]\n", prop.name, prop.value);
}

/* **************************************************
 * Implementations of functions in resetprop.h (APIs)
 * **************************************************/

int prop_exist(const char *name) {
	if (init_resetprop()) return 0;
	return __system_property_find(name) != nullptr;
}

char *getprop(const char *name) {
	return getprop2(name, 0);
}

// Get prop by name, return string (should free manually!)
char *getprop2(const char *name, int persist) {
	if (!check_legal_property_name(name) || init_resetprop())
		return nullptr;
	const prop_info *pi = __system_property_find(name);
	if (pi == nullptr) {
		if (persist && strncmp(name, "persist.", 8) == 0) {
			char *value = persist_getprop(name);
			if (value)
				return value;
		}
		LOGD("resetprop: prop [%s] does not exist\n", name);
		return nullptr;
	} else {
		char value[PROP_VALUE_MAX];
		read_cb_t read_cb;
		read_cb.cb = [](auto, auto value, auto dst) -> void { strcpy((char *) dst, value); };
		read_cb.arg = value;
		read_props(pi, &read_cb);
		LOGD("resetprop: getprop [%s]: [%s]\n", name, value);
		return strdup(value);
	}
}

void getprop_all(void (*callback)(const char *, const char *, void *), void *cookie) {
	if (init_resetprop()) return;
	read_cb_t read_cb(callback, cookie);
	__system_property_foreach(read_props, &read_cb);
}

int setprop(const char *name, const char *value) {
	return setprop2(name, value, 1);
}

int setprop2(const char *name, const char *value, const int trigger) {
	if (!check_legal_property_name(name))
		return 1;
	if (init_resetprop())
		return -1;

	int ret;

	prop_info *pi = (prop_info*) __system_property_find(name);
	if (pi != nullptr) {
		if (trigger) {
			if (strncmp(name, "ro.", 3) == 0) deleteprop(name);
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_update(pi, value, strlen(value));
		}
	} else {
		LOGD("resetprop: New prop [%s]\n", name);
		if (trigger) {
			ret = __system_property_set(name, value);
		} else {
			ret = __system_property_add(name, strlen(name), value, strlen(value));
		}
	}

	LOGD("resetprop: setprop [%s]: [%s] by %s\n", name, value,
		trigger ? "property_service" : "modifing prop data structure");

	if (ret)
		LOGE("resetprop: setprop error\n");

	return ret;
}

int deleteprop(const char *name) {
	return deleteprop2(name, 1);
}

int deleteprop2(const char *name, int persist) {
	if (!check_legal_property_name(name))
		return 1;
	if (init_resetprop()) return -1;
	char path[PATH_MAX];
	path[0] = '\0';
	LOGD("resetprop: deleteprop [%s]\n", name);
	if (persist && strncmp(name, "persist.", 8) == 0)
		persist = persist_deleteprop(name);
	return __system_property_del(name) && !(persist && strncmp(name, "persist.", 8) == 0);
}

int read_prop_file(const char* filename, const int trigger) {
	if (init_resetprop()) return -1;
	LOGD("resetprop: Load prop file [%s]\n", filename);
	FILE *fp = fopen(filename, "r");
	if (fp == nullptr) {
		LOGE("Cannot open [%s]\n", filename);
		return 1;
	}
	char *line = nullptr, *pch;
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
		if ( ((pch == nullptr) || (i >= (pch - line))) || (pch >= line + read - 1) ) continue;
		// Separate the string
		*pch = '\0';
		setprop2(line + i, pch + 1, trigger);
	}
	free(line);
	fclose(fp);
	return 0;
}

int resetprop_main(int argc, char *argv[]) {
	log_cb.d = [](auto fmt, auto ap) -> int { return verbose ? vfprintf(stderr, fmt, ap) : 0; };
	
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
					usage(argv0);
				}
			case 'v':
				verbose = true;
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
				usage(argv0);
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
		if (prop == nullptr) return 1;
		printf("%s\n", prop);
		free(prop);
		return 0;
	case 2:
		return setprop2(argv[0], argv[1], trigger);
	default:
		usage(argv0);
	}
}
