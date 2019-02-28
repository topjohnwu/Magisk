#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <utils.h>
#include <cpio.h>

#include "magiskboot.h"
#include "compress.h"

using namespace std;

static const char *ramdisk_xz = "ramdisk.cpio.xz";

static const char *UNSUPPORT_LIST[] =
		{ "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc",
		  "boot/sbin/launch_daemonsu.sh" };

static const char *MAGISK_LIST[] =
		{ ".backup/.magisk", "init.magisk.rc",
		  "overlay/init.magisk.rc" };

class magisk_cpio : public cpio_rw {
public:
	magisk_cpio() = default;
	explicit magisk_cpio(const char *filename) : cpio_rw(filename) {}
	void patch(bool keepverity, bool keepforceencrypt);
	int test();
	char *sha1();
	void restore();
	void backup(const char *orig);
	void compress();
	void decompress();
};

void magisk_cpio::patch(bool keepverity, bool keepforceencrypt) {
	fprintf(stderr, "Patch with flag KEEPVERITY=[%s] KEEPFORCEENCRYPT=[%s]\n",
			keepverity ? "true" : "false", keepforceencrypt ? "true" : "false");
	auto next = entries.begin();
	decltype(next) cur;
	while (next != entries.end()) {
		cur = next;
		++next;
		bool fstab = (!keepverity || !keepforceencrypt) &&
					 !str_starts(cur->first, ".backup") &&
					 str_contains(cur->first, "fstab") && S_ISREG(cur->second->mode);
		if (!keepverity) {
			if (fstab) {
				patch_verity(&cur->second->data, &cur->second->filesize);
			} else if (cur->first == "verity_key") {
				rm(cur);
				continue;
			}
		}
		if (!keepforceencrypt) {
			if (fstab) {
				patch_encryption(&cur->second->data, &cur->second->filesize);
			}
		}
	}
}

#define STOCK_BOOT        0
#define MAGISK_PATCHED    1 << 0
#define UNSUPPORTED_CPIO  1 << 1
#define COMPRESSED_CPIO   1 << 2
int magisk_cpio::test() {
	if (exists(ramdisk_xz))
		return MAGISK_PATCHED | COMPRESSED_CPIO;

	for (auto file : UNSUPPORT_LIST)
		if (exists(file))
			return UNSUPPORTED_CPIO;

	for (auto file : MAGISK_LIST)
		if (exists(file))
			return MAGISK_PATCHED;

	return STOCK_BOOT;
}

#define for_each_line(line, buf, size) \
for (line = (char *) buf; line < (char *) buf + size && line[0]; line = strchr(line + 1, '\n') + 1)

char *magisk_cpio::sha1() {
	char sha1[41];
	char *line;
	for (auto &e : entries) {
		if (e.first == "init.magisk.rc" || e.first == "overlay/init.magisk.rc") {
			for_each_line(line, e.second->data, e.second->filesize) {
				if (strncmp(line, "#STOCKSHA1=", 11) == 0) {
					strncpy(sha1, line + 12, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (e.first == ".backup/.magisk") {
			for_each_line(line, e.second->data, e.second->filesize) {
				if (strncmp(line, "SHA1=", 5) == 0) {
					strncpy(sha1, line + 5, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (e.first == ".backup/.sha1") {
			return (char *) e.second->data;
		}
	}
	return nullptr;
}

#define for_each_str(str, buf, size) \
for (str = (char *) buf; str < (char *) buf + size; str = str += strlen(str) + 1)

void magisk_cpio::restore() {
	decompress();
	char *file;
	auto next = entries.begin();
	decltype(next) cur;
	while (next != entries.end()) {
		cur = next;
		++next;
		if (str_starts(cur->first, ".backup")) {
			if (cur->first.length() == 7 || cur->first.substr(8) == ".magisk") {
				rm(cur);
			} else if (cur->first.substr(8) == ".rmlist") {
				for_each_str(file, cur->second->data, cur->second->filesize)
					rm(file, false);
				rm(cur);
			} else {
				mv(cur, &cur->first[8]);
			}
		} else if (str_starts(cur->first, "overlay") ||
				str_starts(cur->first, "magisk") ||
				cur->first == "sbin/magic_mask.sh" ||
				cur->first == "init.magisk.rc") {
			// Some known stuff we can remove
			rm(cur);
		}
	}
}

void magisk_cpio::backup(const char *orig) {
	if (access(orig, R_OK))
		return;
	entry_map bkup_entries;
	string remv;

	auto b = new cpio_entry(".backup", S_IFDIR);
	bkup_entries[b->filename].reset(b);

	magisk_cpio o(orig);

	// Remove possible backups in original ramdisk
	o.rm(".backup", true);
	rm(".backup", true);

	auto lhs = o.entries.begin();
	auto rhs = entries.begin();

	while (lhs != o.entries.end() || rhs != entries.end()) {
		int res;
		bool backup = false;
		if (lhs != o.entries.end() && rhs != entries.end()) {
			res = lhs->first.compare(rhs->first);
		} else if (lhs == o.entries.end()) {
			res = 1;
		} else if (rhs == entries.end()) {
			res = -1;
		}

		if (res < 0) {
			// Something is missing in new ramdisk, backup!
			backup = true;
			fprintf(stderr, "Backup missing entry: ");
		} else if (res == 0) {
			if (lhs->second->filesize != rhs->second->filesize ||
				memcmp(lhs->second->data, rhs->second->data, lhs->second->filesize) != 0) {
				// Not the same!
				backup = true;
				fprintf(stderr, "Backup mismatch entry: ");
			}
		} else {
			// Something new in ramdisk
			remv += rhs->first;
			remv += (char) '\0';
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", rhs->first.data());
		}
		if (backup) {
			string back_name(".backup/");
			back_name += lhs->first;
			fprintf(stderr, "[%s] -> [%s]\n", lhs->first.data(), back_name.data());
			auto ex = static_cast<cpio_entry*>(lhs->second.release());
			ex->filename = back_name;
			bkup_entries[ex->filename].reset(ex);
		}

		// Increment positions
		if (res < 0) {
			++lhs;
		} else if (res == 0) {
			++lhs; ++rhs;
		} else {
			++rhs;
		}
	}

	if (!remv.empty()) {
		auto rmlist = new cpio_entry(".backup/.rmlist", S_IFREG);
		rmlist->filesize = remv.length();
		rmlist->data = xmalloc(remv.length());
		memcpy(rmlist->data, remv.data(), remv.length());
		bkup_entries[rmlist->filename].reset(rmlist);
	}

	if (bkup_entries.size() > 1)
		entries.merge(bkup_entries);
}

void magisk_cpio::compress() {
	if (exists(ramdisk_xz))
		return;
	fprintf(stderr, "Compressing cpio -> [%s]\n", ramdisk_xz);
	auto init = entries.extract("init");
	XZEncoder encoder;
	encoder.set_out(make_unique<BufOutStream>());
	output(encoder);
	encoder.finalize();
	auto backup = entries.extract(".backup");
	auto config = entries.extract(".backup/.magisk");
	entries.clear();
	entries.insert(std::move(init));
	entries.insert(std::move(backup));
	entries.insert(std::move(config));
	auto xz = new cpio_entry(ramdisk_xz, S_IFREG);
	static_cast<BufOutStream *>(encoder.get_out())->release(xz->data, xz->filesize);
	insert(xz);
}

void magisk_cpio::decompress() {
	auto it = entries.find(ramdisk_xz);
	if (it == entries.end())
		return;
	fprintf(stderr, "Decompressing cpio [%s]\n", ramdisk_xz);
	entries.erase(".backup");
	entries.erase(".backup/.magisk");
	LZMADecoder decoder;
	decoder.set_out(make_unique<BufOutStream>());
	decoder.write(it->second->data, it->second->filesize);
	decoder.finalize();
	entries.erase(it);
	char *buf;
	size_t sz;
	static_cast<BufOutStream *>(decoder.get_out())->getbuf(buf, sz);
	load_cpio(buf, sz);
}

int cpio_commands(int argc, char *argv[]) {
	char *incpio = argv[0];
	++argv;
	--argc;

	magisk_cpio cpio;
	if (access(incpio, R_OK) == 0)
		cpio.load_cpio(incpio);

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
		} else if (strcmp(cmdv[0], "compress") == 0){
			cpio.compress();
		} else if (strcmp(cmdv[0], "decompress") == 0){
			cpio.decompress();
		} else if (cmdc == 2 && strcmp(cmdv[0], "exists") == 0) {
			exit(!cpio.exists(cmdv[1]));
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
			cpio.mkdir(strtoul(cmdv[1], nullptr, 8), cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "ln") == 0) {
			cpio.ln(cmdv[1], cmdv[2]);
		} else if (cmdc == 4 && strcmp(cmdv[0], "add") == 0) {
			cpio.add(strtoul(cmdv[1], nullptr, 8), cmdv[2], cmdv[3]);
		} else {
			return 1;
		}

		--argc;
		++argv;
	}

	cpio.dump(incpio);
	return 0;
}