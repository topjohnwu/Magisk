#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "magiskboot.h"
#include "array.h"
#include "cpio.h"
#include "utils.h"

class magisk_cpio : public cpio {
public:
	magisk_cpio(const char *filename) : cpio(filename) {}
	void patch(bool keepverity, bool keepforceencrypt);
	int test();
	char * sha1();
	void restore();
	void backup(Array<cpio_entry*> &bak, const char *orig, const char *sha1);
};

void magisk_cpio::patch(bool keepverity, bool keepforceencrypt) {
	fprintf(stderr, "Patch with flag KEEPVERITY=[%s] KEEPFORCEENCRYPT=[%s]\n",
			keepverity ? "true" : "false", keepforceencrypt ? "true" : "false");
	for (auto &e : arr) {
		if (!e)
			continue;
		if (!keepverity) {
			if (strncmp(e->filename, ".backup", 7) && strstr(e->filename, "fstab") && S_ISREG(e->mode)) {
				patch_verity(&e->data, &e->filesize, 1);
			} else if (strcmp(e->filename, "verity_key") == 0) {
				fprintf(stderr, "Remove [verity_key]\n");
				delete e;
				e = nullptr;
				continue;
			}
		}
		if (!keepforceencrypt) {
			if (strstr(e->filename, "fstab") != NULL && S_ISREG(e->mode)) {
				patch_encryption(&e->data, &e->filesize);
			}
		}
	}
}


#define STOCK_BOOT       0x0
#define MAGISK_PATCH     0x1
#define UNSUPPORT_PATCH  0x2
int magisk_cpio::test() {
	static const char *UNSUPPORT_LIST[] = { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc",
										 "boot/sbin/launch_daemonsu.sh", nullptr };
	static const char *MAGISK_LIST[] = { ".backup/.magisk", "init.magisk.rc",
									  "overlay/init.magisk.rc", nullptr };

	for (int i = 0; UNSUPPORT_LIST[i]; ++i)
		if (find(UNSUPPORT_LIST[i]) >= 0)
			return UNSUPPORT_PATCH;

	for (int i = 0; MAGISK_LIST[i]; ++i)
		if (find(MAGISK_LIST[i]) >= 0)
			return MAGISK_PATCH;

	return STOCK_BOOT;
}

char * magisk_cpio::sha1() {
	char sha1[41];
	for (auto &e : arr) {
		if (!e) continue;
		if (strcmp(e->filename, "init.magisk.rc") == 0
			|| strcmp(e->filename, "overlay/init.magisk.rc") == 0) {
			for (char *pos = (char *) e->data; pos < (char *) e->data + e->filesize;
					pos = strchr(pos + 1, '\n') + 1) {
				if (memcmp(pos, "# STOCKSHA1=", 12) == 0) {
					pos += 12;
					memcpy(sha1, pos, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (strcmp(e->filename, ".backup/.sha1") == 0) {
			return (char *) e->data;
		}
	}
	return nullptr;
}

void magisk_cpio::restore() {
	for (auto &e : arr) {
		if (!e) continue;
		if (strncmp(e->filename, ".backup", 7) == 0) {
			if (e->filename[7] == '\0') continue;
			if (e->filename[8] == '.') {
				if (strcmp(e->filename + 8, ".rmlist") == 0) {
					for (int pos = 0; pos < e->filesize; pos += strlen((char *) e->data + pos) + 1)
						rm(false, (char *) e->data + pos);
				}
			} else {
				mv(e->filename, e->filename + 8);
			}
		}
	}

	// Some known stuff we can remove
	rm(true, ".backup");
	rm(true, "overlay");
	rm(false, "sbin/magic_mask.sh");
	rm(false, "init.magisk.rc");
	rm(true, "magisk");
}

void magisk_cpio::backup(Array<cpio_entry*> &bak, const char *orig, const char *sha1) {
	cpio_entry *m, *n, *rem, *cksm;
	char buf[PATH_MAX];
	int res;
	bool backup;

	m = new cpio_entry();
	m->filename = strdup(".backup");
	m->mode = S_IFDIR;
	bak.push_back(m);

	rem = new cpio_entry();
	rem->filename = strdup(".backup/.rmlist");
	rem->mode = S_IFREG;

	if (sha1) {
		fprintf(stderr, "Save SHA1: [%s] -> [.backup/.sha1]\n", sha1);
		cksm = new cpio_entry();
		bak.push_back(cksm);
		cksm->filename = strdup(".backup/.sha1");
		cksm->mode = S_IFREG;
		cksm->data = strdup(sha1);
		cksm->filesize = strlen(sha1) + 1;
	}

	magisk_cpio o = magisk_cpio(orig);

	// Remove possible backups in original ramdisk
	o.rm(true, ".backup");
	rm(true, ".backup");

	// Sort both CPIOs before comparing
	o.sort();
	sort();

	// Start comparing
	size_t i = 0, j = 0;
	while(i != o.arr.size() || j != arr.size()) {
		backup = false;
		if (i != o.arr.size() && j != arr.size()) {
			m = o.arr[i];
			n = arr[j];
			res = strcmp(m->filename, n->filename);
		} else if (i == o.arr.size()) {
			n = arr[j];
			res = 1;
		} else if (j == arr.size()) {
			m = o.arr[i];
			res = -1;
		}

		if (res < 0) {
			// Something is missing in new ramdisk, backup!
			++i;
			backup = true;
			fprintf(stderr, "Backup missing entry: ");
		} else if (res == 0) {
			++i; ++j;
			if (m->filesize == n->filesize && memcmp(m->data, n->data, m->filesize) == 0)
				continue;
			// Not the same!
			backup = true;
			fprintf(stderr, "Backup mismatch entry: ");
		} else {
			// Something new in ramdisk, record in rem
			++j;
			rem->data = xrealloc(rem->data, rem->filesize + strlen(n->filename) + 1);
			memcpy((char *) rem->data + rem->filesize, n->filename, strlen(n->filename) + 1);
			rem->filesize += strlen(n->filename) + 1;
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", n->filename);
		}
		if (backup) {
			sprintf(buf, ".backup/%s", m->filename);
			fprintf(stderr, "[%s] -> [%s]\n", m->filename, buf);
			free(m->filename);
			m->filename = strdup(buf);
			bak.push_back(m);
			// NULL the original entry, so it won't be freed
			o.arr[i - 1] = nullptr;
		}
	}

	if (rem->filesize)
		bak.push_back(rem);
	else
		delete rem;
}


int cpio_commands(int argc, char *argv[]) {
	char *incpio = argv[0];
	++argv;
	--argc;

	magisk_cpio cpio = magisk_cpio(incpio);

	int cmdc;
	char *cmdv[6];

	while (argc) {
		// Clean up
		cmdc = 0;
		memset(cmdv, NULL, sizeof(cmdv));

		// Split the commands
		for (char *tok = strtok(argv[0], " "); tok; tok = strtok(nullptr, " "))
			cmdv[cmdc++] = tok;

		if (cmdc == 0)
			continue;

		if (strcmp(cmdv[0], "test") == 0) {
			exit(cpio.test());
		} else if (strcmp(cmdv[0], "restore") == 0) {
			cpio.restore();
		} else if (strcmp(cmdv[0], "sha1") == 0) {
			char *sha1 = cpio.sha1();
			if (sha1)
				printf("%s\n", sha1);
			free(sha1);
			return 0;
		} else if (cmdc >= 2 && strcmp(cmdv[0], "backup") == 0) {
			auto bak = Array<cpio_entry*>();
			cpio.backup(bak, cmdv[1], cmdv[2]);
			cpio.insert(bak);
		} else if (cmdc >= 4 && strcmp(cmdv[0], "magisk") == 0) {
			cpio.patch(strcmp(cmdv[2], "true") == 0, strcmp(cmdv[3], "true") == 0);

			auto bak = Array<cpio_entry*>();
			cpio.backup(bak, cmdv[1], cmdv[4]);

			auto e = new cpio_entry();
			e->filename = strdup(".backup/.magisk");
			e->mode = S_IFREG;
			e->data = xmalloc(50);
			snprintf((char *) e->data, 50, "KEEPVERITY=%s\nKEEPFORCEENCRYPT=%s\n", cmdv[2], cmdv[3]);
			e->filesize = strlen((char *) e->data) + 1;

			cpio.insert(bak);
			cpio.insert(e);
		} else if (cmdc >= 2 && strcmp(cmdv[0], "rm") == 0) {
			int recur = cmdc > 2 && strcmp(cmdv[1], "-r") == 0;
			cpio.rm(recur, cmdv[1 + recur]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "mv") == 0) {
			cpio.mv(cmdv[1], cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "patch") == 0) {
			cpio.patch(strcmp(cmdv[1], "true") == 0, strcmp(cmdv[2], "true") == 0);
		} else if (strcmp(cmdv[0], "extract") == 0) {
			if (cmdc == 3) {
				return cpio.extract(cmdv[1], cmdv[2]);
			} else {
				cpio.extract();
				return 0;
			}
		} else if (cmdc == 3 && strcmp(cmdv[0], "mkdir") == 0) {
			cpio.makedir(strtoul(cmdv[1], NULL, 8), cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "ln") == 0) {
			cpio.ln(cmdv[1], cmdv[2]);
		} else if (cmdc == 4 && strcmp(cmdv[0], "add") == 0) {
			cpio.add(strtoul(cmdv[1], NULL, 8), cmdv[2], cmdv[3]);
		} else {
			return 1;
		}

		--argc;
		++argv;
	}

	cpio.dump(incpio);
	return 0;
}