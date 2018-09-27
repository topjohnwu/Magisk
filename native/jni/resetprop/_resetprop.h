/* resetprop.h - Internal struct definitions
 */


#ifndef MAGISK_PROPS_H
#define MAGISK_PROPS_H

#include "resetprop/private/system_properties.h"
#include "logging.h"

extern int prop_verbose;

struct prop_t {
	char *name;
	char value[PROP_VALUE_MAX];
};

struct read_cb_t {
	void (*func)(const char *name, const char *value, void *cookie);
	void *cookie;
};

char *persist_getprop(const char *name);
void persist_getprop_all(struct read_cb_t *read_cb);
bool persist_deleteprop(const char *name);
void collect_props(const char *name, const char *value, void *prop_list);

#endif //MAGISK_PROPS_H
