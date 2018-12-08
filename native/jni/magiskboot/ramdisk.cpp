#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "magiskboot.h"
#include "cpio.h"
#include "utils.h"

class magisk_cpio : public cpio {
public:
	magisk_cpio(const char *filename) : cpio(filename) {}
	void patch(bool keepverity, bool keepforceencrypt);
	int test();
	char * sha1();
	void restore();
	void backup(const char *orig);
};

void magisk_cpio::patch(bool keepverity, bool keepforceencrypt) {
	fprintf(stderr, "Patch with flag KEEPVERITY=[%s] KEEPFORCEENCRYPT=[%s]\n",
			keepverity ? "true" : "false", keepforceencrypt ? "true" : "false");
	for (auto &e : arr) {
		if (!e)
			continue;
		bool fstab = (!keepverity || !keepforceencrypt) &&
				!e->filename.starts_with(".backup") &&
					 e->filename.contains("fstab") && S_ISREG(e->mode);
		if (!keepverity) {
			if (fstab) {
				patch_verity(&e->data, &e->filesize, 1);
			} else if (e->filename == "verity_key") {
				fprintf(stderr, "Remove [verity_key]\n");
				delete e;
				e = nullptr;
				continue;
			}
		}
		if (!keepforceencrypt) {
			if (fstab) {
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
										 "boot/sbin/launch_daemonsu.sh" };
	static const char *MAGISK_LIST[] = { ".backup/.magisk", "init.magisk.rc",
									  "overlay/init.magisk.rc" };

	for (auto file : UNSUPPORT_LIST)
		if (find(file) >= 0)
			return UNSUPPORT_PATCH;

	for (auto file : MAGISK_LIST)
		if (find(file) >= 0)
			return MAGISK_PATCH;

	return STOCK_BOOT;
}

#define for_each_line(line, buf, size) \
for (line = (char *) buf; line < (char *) buf + size && line[0]; line = strchr(line + 1, '\n') + 1)

char *magisk_cpio::sha1() {
	char sha1[41];
	char *line;
	for (auto &e : arr) {
		if (!e) continue;
		if (e->filename == "init.magisk.rc" || e->filename == "overlay/init.magisk.rc") {
			for_each_line(line, e->data, e->filesize) {
				if (strncmp(line, "#STOCKSHA1=", 11) == 0) {
					strncpy(sha1, line + 12, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (e->filename == ".backup/.magisk") {
			for_each_line(line, e->data, e->filesize) {
				if (strncmp(line, "SHA1=", 5) == 0) {
					strncpy(sha1, line + 5, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (e->filename == ".backup/.sha1") {
			return (char *) e->data;
		}
	}
	return nullptr;
}

void magisk_cpio::restore() {
	for (auto &e : arr) {
		if (!e) continue;
		if (e->filename.starts_with(".backup")) {
			if (e->filename[7] == '\0') continue;
			if (e->filename[8] == '.') {
				if (strcmp(&e->filename[8], ".rmlist") == 0) {
					for (int pos = 0; pos < e->filesize; pos += strlen((char *) e->data + pos) + 1)
						rm((char *) e->data + pos, false);
				}
			} else {
				mv(e->filename, e->filename + 8);
			}
		}
	}

	// Some known stuff we can remove
	rm(".backup", true);
	rm("overlay", true);
	rm("sbin/magic_mask.sh", false);
	rm("init.magisk.rc", false);
	rm("magisk", true);
}

void magisk_cpio::backup(const char *orig) {
	Vector<cpio_entry*> bak;
	cpio_entry *m, *n, *rem;
	char buf[PATH_MAX];

	m = new cpio_entry();
	m->filename = ".backup";
	m->mode = S_IFDIR;
	bak.push_back(m);

	rem = new cpio_entry();
	rem->filename = ".backup/.rmlist";
	rem->mode = S_IFREG;

	magisk_cpio o(orig);

	// Remove possible backups in original ramdisk
	o.rm(".backup", true);
	rm(".backup", true);

	// Sort both CPIOs before comparing
	o.sort();
	sort();

	// Start comparing
	size_t i = 0, j = 0;
	while(i != o.arr.size() || j != arr.size()) {
		int res;
		bool backup = false;
		if (i != o.arr.size() && j != arr.size()) {
			m = o.arr[i];
			n = arr[j];
			res = m->filename.compare(n->filename);
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
			rem->data = xrealloc(rem->data, rem->filesize + n->filename.size());
			memcpy((char *) rem->data + rem->filesize, n->filename, n->filename.size());
			rem->filesize += n->filename.size();
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", n->filename.c_str());
		}
		if (backup) {
			sprintf(buf, ".backup/%s", m->filename.c_str());
			fprintf(stderr, "[%s] -> [%s]\n", m->filename.c_str(), buf);
			m->filename = buf;
			bak.push_back(m);
			// NULL the original entry, so it won't be freed
			o.arr[i - 1] = nullptr;
		}
	}

	if (rem->filesize)
		bak.push_back(rem);
	else
		delete rem;

	if (bak.size() > 1)
		insert(bak);
}


int cpio_commands(int argc, char *argv[]) {
	char *incpio = argv[0];
	++argv;
	--argc;

	magisk_cpio cpio(incpio);

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
			if (sha1) printf("%s\n", sha1);
			return 0;
		} else if (cmdc == 2 && strcmp(cmdv[0], "exists") == 0) {
			exit(cpio.find(cmdv[1]) < 0);
		} else if (cmdc == 2 && strcmp(cmdv[0], "backup") == 0) {
			cpio.backup(cmdv[1]);
		} else if (cmdc >= 2 && strcmp(cmdv[0], "rm") == 0) {
			bool r = cmdc > 2 && strcmp(cmdv[1], "-r") == 0;
			cpio.rm(cmdv[1 + r], r);
		} else if (cmdc == 3 && strcmp(cmdv[0], "mv") == 0) {
			cpio.mv(cmdv[1], cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "patch") == 0) {
			cpio.patch(strcmp(cmdv[1], "true") == 0, strcmp(cmdv[2], "true") == 0);
		} else if (strcmp(cmdv[0], "extract") == 0) {
			if (cmdc == 3) {
				return !cpio.extract(cmdv[1], cmdv[2]);
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