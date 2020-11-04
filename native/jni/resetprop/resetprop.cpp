#include <dlfcn.h>
#include <sys/types.h>
#include <vector>
#include <map>

#include <resetprop.hpp>
#include <utils.hpp>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <_system_properties.h>

#include "_resetprop.hpp"

using namespace std;

static bool verbose = false;

static int (*system_property_get)(const char*, char*);
static int (*system_property_set)(const char*, const char*);

[[noreturn]] static void usage(char* arg0) {
	fprintf(stderr,
R"EOF(resetprop - System Property Manipulation Tool

Usage: %s [flags] [options...]

Options:
   -h, --help        show this message
   (no arguments)    print all properties
   NAME              get property
   NAME VALUE        set property entry NAME with VALUE
   --file FILE       load props from FILE
   --delete NAME     delete property

Flags:
   -v      print verbose output to stderr
   -n      set props without going through property_service
           (this flag only affects setprop)
   -p      read/write props from/to persistent storage
           (this flag only affects getprop and delprop)

)EOF", arg0);
	exit(1);
}

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

static void read_prop(const prop_info *pi, void *cb) {
	auto callback = [](void *cb, const char *name, const char *value, uint32_t) {
		static_cast<prop_cb*>(cb)->exec(name, value);
	};
	__system_property_read_callback(pi, callback, cb);
}

static void print_props(bool persist) {
	getprops([](const char *name, const char *value, auto) {
		printf("[%s]: [%s]\n", name, value);
	}, nullptr, persist);
}

struct sysprop_stub {
	virtual int setprop(const char *name, const char *value, bool trigger) { return 1; }
	virtual string getprop(const char *name, bool persist) { return ""; }
	virtual void getprops(void (*callback)(const char *, const char *, void *),
			void *cookie, bool persist) {}
	virtual int delprop(const char *name, bool persist) { return 1; }
};

struct sysprop : public sysprop_stub {
	int setprop(const char *name, const char *value, bool) override {
		return system_property_set(name, value);
	}
	string getprop(const char *name, bool) override {
		char buf[PROP_VALUE_MAX];
		buf[0] = '\0';
		system_property_get(name, buf);
		return buf;
	}
};

struct resetprop : public sysprop_stub {
	int setprop(const char *name, const char *value, bool prop_svc) override {
		if (!check_legal_property_name(name))
			return 1;

		const char *msg = prop_svc ? "property_service" : "modifying prop data structure";
		int ret;
		auto pi = const_cast<prop_info *>(__system_property_find(name));
		if (pi != nullptr) {
			if (prop_svc) {
				if (strncmp(name, "ro.", 3) == 0)
					delprop(name, false);
				ret = system_property_set(name, value);
			} else {
				ret = __system_property_update(pi, value, strlen(value));
			}
			LOGD("resetprop: update prop [%s]: [%s] by %s\n", name, value, msg);
		} else {
			if (prop_svc) {
				ret = system_property_set(name, value);
			} else {
				ret = __system_property_add(name, strlen(name), value, strlen(value));
			}
			LOGD("resetprop: create prop [%s]: [%s] by %s\n", name, value, msg);
		}

		if (ret)
			LOGE("resetprop: setprop error\n");

		return ret;
	}

	struct prop_to_string : prop_cb {
		explicit prop_to_string(string &s) : val(s) {}
		void exec(const char *name, const char *value) override {
			val = value;
		}
	private:
		string &val;
	};

	string getprop(const char *name, bool persist) override {
		if (!check_legal_property_name(name))
			return string();
		auto pi = __system_property_find(name);
		if (pi == nullptr) {
			if (persist && strncmp(name, "persist.", 8) == 0) {
				if (auto value = persist_getprop(name); !value.empty())
					return value;
			}
			LOGD("resetprop: prop [%s] does not exist\n", name);
			return string();
		} else {
			string val;
			auto prop = prop_to_string(val);
			read_prop(pi, &prop);
			LOGD("resetprop: getprop [%s]: [%s]\n", name, val.data());
			return val;
		}
	}

	void getprops(void (*callback)(const char *, const char *, void *),
			void *cookie, bool persist) override {
		prop_list list;
		prop_collector collector(list);
		__system_property_foreach(read_prop, &collector);
		if (persist)
			persist_getprops(&collector);
		for (auto &[key, val] : list)
			callback(key.data(), val.data(), cookie);
	}

	int delprop(const char *name, bool persist) override {
		if (!check_legal_property_name(name))
			return 1;
		LOGD("resetprop: delete prop [%s]\n", name);
		if (persist && strncmp(name, "persist.", 8) == 0)
			persist = persist_deleteprop(name);
		return __system_property_delete(name) && !(persist && strncmp(name, "persist.", 8) == 0);
	}
};

#define DLOAD(name) \
*(void **) &name = dlsym(RTLD_DEFAULT, "__" #name)

static sysprop_stub *get_impl() {
	static sysprop_stub *impl = nullptr;
	if (impl == nullptr) {
		use_pb = access(PERSISTENT_PROPERTY_DIR "/persistent_properties", R_OK) == 0;
#ifdef APPLET_STUB_MAIN
		if (__system_properties_init()) {
			LOGE("resetprop: __system_properties_init error\n");
			exit(1);
		}
		impl = new resetprop();
		system_property_set = &__system_property_set;
#else
		DLOAD(system_property_set);  /* Always prefer to use setprop from platform */
		if (__system_properties_init()) {
			LOGW("resetprop: __system_properties_init error\n");
			DLOAD(system_property_get);
			impl = system_property_get ? new sysprop() : new sysprop_stub();
		} else {
			impl = new resetprop();
		}
#endif
	}
	return impl;
}

/*********************************
 * Implementation of public APIs
 *********************************/

string getprop(const char *name, bool persist) {
	return get_impl()->getprop(name, persist);
}

void getprops(void (*callback)(const char *, const char *, void *), void *cookie, bool persist) {
	get_impl()->getprops(callback, cookie, persist);
}

int setprop(const char *name, const char *value, bool trigger) {
	return get_impl()->setprop(name, value, trigger);
}

int delprop(const char *name, bool persist) {
	return get_impl()->delprop(name, persist);
}

void load_prop_file(const char *filename, bool prop_svc) {
	auto impl = get_impl();
	LOGD("resetprop: Parse prop file [%s]\n", filename);
	parse_prop_file(filename, [=](auto key, auto val) -> bool {
		impl->setprop(key.data(), val.data(), prop_svc);
		return true;
	});
}

int resetprop_main(int argc, char *argv[]) {
	log_cb.d = [](auto fmt, auto ap) -> int { return verbose ? vfprintf(stderr, fmt, ap) : 0; };

	bool prop_svc = true;
	bool persist = false;
	char *argv0 = argv[0];

	--argc;
	++argv;

	// Parse flags and -- options
	while (argc && argv[0][0] == '-') {
		for (int idx = 1; true; ++idx) {
			switch (argv[0][idx]) {
			case '-':
				if (strcmp(argv[0], "--file") == 0 && argc == 2) {
					load_prop_file(argv[1], prop_svc);
					return 0;
				} else if (strcmp(argv[0], "--delete") == 0 && argc == 2) {
					return delprop(argv[1], persist);
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
				prop_svc = false;
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
	case 1: {
		string prop = getprop(argv[0], persist);
		if (prop.empty())
			return 1;
		printf("%s\n", prop.data());
		return 0;
	}
	case 2:
		return setprop(argv[0], argv[1], prop_svc);
	default:
		usage(argv0);
	}
}
