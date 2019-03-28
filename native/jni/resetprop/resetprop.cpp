#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <algorithm>

#include <magisk.h>
#include <resetprop.h>
#include <utils.h>
#include <flags.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include "private/_system_properties.h"
#include "private/system_properties.h"
#include "_resetprop.h"

using namespace std;

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
		FULL_VER(resetprop) " - System Props Modification Tool\n\n"
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

static void read_props(const prop_info *pi, void *read_cb) {
	__system_property_read_callback(
			pi, [](auto cb, auto name, auto value, auto) -> void
			{
				((read_cb_t *) cb)->exec(name, value);
			}, read_cb);
}

void collect_props(const char *name, const char *value, void *v_plist) {
	auto &prop_list = *static_cast<vector<prop_t> *>(v_plist);
	prop_list.emplace_back(name, value);
}

static void collect_unique_props(const char *name, const char *value, void *v_plist) {
	auto &prop_list = *static_cast<vector<prop_t> *>(v_plist);
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

static void print_props(bool persist) {
	vector<prop_t> prop_list;
	getprop(collect_props, &prop_list, persist);
	sort(prop_list.begin(), prop_list.end());
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

// Get prop by name, return string
string getprop(const char *name, bool persist) {
	if (!check_legal_property_name(name) || init_resetprop())
		return string();
	const prop_info *pi = __system_property_find(name);
	if (pi == nullptr) {
		if (persist && strncmp(name, "persist.", 8) == 0) {
			auto value = persist_getprop(name);
			if (value.empty())
				LOGD("resetprop: prop [%s] does not exist\n", name);
			return value;
		}
		return string();
	} else {
		char value[PROP_VALUE_MAX];
		read_cb_t read_cb;
		read_cb.cb = [](auto, auto value, auto dst) -> void { strcpy((char *) dst, value); };
		read_cb.arg = value;
		read_props(pi, &read_cb);
		LOGD("resetprop: getprop [%s]: [%s]\n", name, value);
		return value;
	}
}

void getprop(void (*callback)(const char *, const char *, void *), void *cookie, bool persist) {
	if (init_resetprop()) return;
	read_cb_t read_cb(callback, cookie);
	__system_property_foreach(read_props, &read_cb);
	if (persist) {
		read_cb.cb = collect_unique_props;
		persist_getprop(&read_cb);
	}
}

int setprop(const char *name, const char *value, bool trigger) {
	if (!check_legal_property_name(name))
		return 1;
	if (init_resetprop())
		return -1;

	int ret;

	auto pi = (prop_info*) __system_property_find(name);
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

int deleteprop(const char *name, bool persist) {
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

void load_prop_file(const char *filename, bool trigger) {
	if (init_resetprop()) return;
	LOGD("resetprop: Parse prop file [%s]\n", filename);
	parse_prop_file(filename, [=](auto key, auto val) -> bool {
		setprop(key.data(), val.data(), trigger);
		return true;
	});
}

int resetprop_main(int argc, char *argv[]) {
	log_cb.d = [](auto fmt, auto ap) -> int { return verbose ? vfprintf(stderr, fmt, ap) : 0; };

	bool trigger = true, persist = false;
	char *argv0 = argv[0];
	string prop;

	--argc;
	++argv;

	// Parse flags and -- options
	while (argc && argv[0][0] == '-') {
		for (int idx = 1; true; ++idx) {
			switch (argv[0][idx]) {
			case '-':
				if (strcmp(argv[0], "--file") == 0 && argc == 2) {
					load_prop_file(argv[1], trigger);
					return 0;
				} else if (strcmp(argv[0], "--delete") == 0 && argc == 2) {
					return deleteprop(argv[1], persist);
				} else if (strcmp(argv[0], "--help") == 0) {
					usage(argv0);
				}
			case 'v':
				verbose = true;
				continue;
			case 'p':
				persist = true;
				continue;
			case 'n':
				trigger = false;
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
		prop = getprop(argv[0], persist);
		if (prop.empty()) return 1;
		printf("%s\n", prop.c_str());
		return 0;
	case 2:
		return setprop(argv[0], argv[1], trigger);
	default:
		usage(argv0);
	}
}
