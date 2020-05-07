#pragma once

#include <string>
#include <map>
#include <logging.hpp>

#include <system_properties.h>

#define PERSISTENT_PROPERTY_DIR  "/data/property"

struct prop_cb {
	virtual void exec(const char *name, const char *value) = 0;
};

template<class T>
struct prop_cb_impl : public prop_cb {
	typedef void (*callback_type)(const char *, const char *, T&);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-field"  // Dangling field is expected
	prop_cb_impl(T &arg, callback_type fn) : arg(arg), fn(fn) {}
#pragma GCC diagnostic pop

	void exec(const char *name, const char *value) override {
		fn(name, value, arg);
	}
private:
	T &arg;
	callback_type fn;
};

extern bool use_pb;

using prop_list = std::map<std::string_view, std::string>;

struct prop_collector : prop_cb_impl<prop_list> {
	prop_collector(prop_list &list) :
	prop_cb_impl<prop_list>(list, [](auto name, auto val, auto list){ list.emplace(name, val); }) {}
};

template <class T, class Func>
prop_cb_impl<T> make_prop_cb(T &arg, Func f) {
	return prop_cb_impl<T>(arg, f);
}

std::string persist_getprop(const char *name);
void persist_getprops(prop_cb *prop_cb);
bool persist_deleteprop(const char *name);
void persist_cleanup();
