#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "su.h"
#include "utils.h"

enum {
	H_NO_CONTEXT = 0x0001,
};

static struct {
	const char *package;
	int flags;
	int uid;
} apps_list[] = {
	{ "com.keramidas.TitaniumBackup", H_NO_CONTEXT, },
};

void hacks_init() {
	char oldCwd[512];
	int i;
	getcwd(oldCwd, sizeof(oldCwd));
	chdir("/data/data");
	for(i=0; i<(sizeof(apps_list)/sizeof(apps_list[0])); ++i) {
		apps_list[i].uid = -1;
		struct stat st_buf;
		int ret = stat(apps_list[i].package, &st_buf);
		LOGW("hacks: Testing (%s:%d:%d)", apps_list[i].package, ret, st_buf.st_uid);
		if(ret)
			continue;
		apps_list[i].uid = st_buf.st_uid;
	}
}

void hacks_update_context(struct su_context* ctxt) {
	int i;
	for(i=0; i<(sizeof(apps_list)/sizeof(apps_list[0])); ++i) {
		LOGW("hacks: Testing (%s:%d), %d", apps_list[i].package, ctxt->from.uid);
		if(apps_list[i].uid != ctxt->from.uid)
			continue;

		LOGW("hacks: Found app (%s:%d)", apps_list[i].package, ctxt->from.uid);
		if(apps_list[i].flags & H_NO_CONTEXT) {
			LOGW("hacks: Disabling context (%s:%d)", apps_list[i].package, ctxt->from.uid);
			ctxt->to.context = NULL;
		}
	}
}
