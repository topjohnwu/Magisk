#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "cpio.h"
#include "utils.h"
#include "logging.h"

#define parse_align() lseek(fd, align(lseek(fd, 0, SEEK_CUR), 4), SEEK_SET)

static uint32_t x8u(char *hex) {
	uint32_t val, inpos = 8, outpos;
	char pattern[6];

	while (*hex == '0') {
		hex++;
		if (!--inpos) return 0;
	}
	// Because scanf gratuitously treats %*X differently than printf does.
	sprintf(pattern, "%%%dx%%n", inpos);
	sscanf(hex, pattern, &val, &outpos);
	if (inpos != outpos)
		LOGE("bad cpio header\n");

	return val;
}

cpio_entry::cpio_entry(int fd, cpio_newc_header &header) {
	// ino = x8u(header.ino);
	mode = x8u(header.mode);
	uid = x8u(header.uid);
	gid = x8u(header.gid);
	// nlink = x8u(header.nlink);
	// mtime = x8u(header.mtime);
	filesize = x8u(header.filesize);
	// devmajor = x8u(header.devmajor);
	// devminor = x8u(header.devminor);
	// rdevmajor = x8u(header.rdevmajor);
	// rdevminor = x8u(header.rdevminor);
	uint32_t namesize = x8u(header.namesize);
	// check = x8u(header.check);
	filename = (char *) xmalloc(namesize);
	xxread(fd, filename, namesize);
	parse_align();
	if (filesize) {
		data = xmalloc(filesize);
		xxread(fd, data, filesize);
		parse_align();
	}
}

cpio_entry::~cpio_entry() {
	free(filename);
	free(data);
}

// Define the way to sort cpio_entry
template<>
int(*Array<cpio_entry*>::_cmp)(cpio_entry*&, cpio_entry*&) = [](auto a, auto b) -> int {
	if (a == b) return 0;
	if (a == nullptr) return 1;
	if (b == nullptr) return -1;
	return strcmp(a->filename, b->filename);
};


cpio::cpio(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return;
	fprintf(stderr, "Loading cpio: [%s]\n", filename);
	cpio_newc_header header;
	cpio_entry *entry;
	while(xxread(fd, &header, sizeof(cpio_newc_header)) != -1) {
		entry = new cpio_entry(fd, header);
		if (strcmp(entry->filename, ".") == 0 || strcmp(entry->filename, "..") == 0 ||
			strcmp(entry->filename, "TRAILER!!!") == 0) {
			delete entry;
			if (entry->filename[0] == 'T')
				break;
			continue;
		}
		arr.push_back(entry);
	}
	close(fd);
}

cpio::~cpio() {
	for (auto &e : arr)
		if (e) delete e;
}

#define dump_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), 4))
void cpio::dump(const char *file) {
	fprintf(stderr, "Dump cpio: [%s]\n", file);
	unsigned inode = 300000;
	char header[111];
	sort();
	int fd = creat(file, 0644);
	for (auto &e : arr) {
		sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
				inode++,    // e->ino
				e->mode,
				e->uid,
				e->gid,
				1,          // e->nlink
				0,          // e->mtime
				e->filesize,
				0,          // e->devmajor
				0,          // e->devminor
				0,          // e->rdevmajor
				0,          // e->rdevminor
				(uint32_t) strlen(e->filename) + 1,
				0           // e->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, e->filename, strlen(e->filename) + 1);
		dump_align();
		if (e->filesize) {
			xwrite(fd, e->data, e->filesize);
			dump_align();
		}
	}
	// Write trailer
	sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
			inode++, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
	xwrite(fd, header, 110);
	xwrite(fd, "TRAILER!!!\0", 11);
	dump_align();
	close(fd);
}

int cpio::find(const char *name) {
	for (int i = 0; i < arr.size(); ++i) {
		if (!arr[i])
			continue;
		if (strcmp(arr[i]->filename, name) == 0)
			return i;
	}
	return -1;
}

void cpio::insert(cpio_entry *e) {
	int i = find(e->filename);
	if (i >= 0) {
		// Replace, then all is done
		delete arr[i];
		arr[i] = e;
		return;
	}
	arr.push_back(e);
}

void cpio::insert(Array<cpio_entry *> &arr) {
	for (auto &e : arr)
		insert(e);
}

void cpio::rm(int recur, const char *name) {
	size_t len = strlen(name);
	for (auto &e : arr) {
		if (!e)
			continue;
		if (strncmp(e->filename, name, len) == 0 &&
			((recur && e->filename[len] == '/') || e->filename[len] == '\0')) {
			fprintf(stderr, "Remove [%s]\n", e->filename);
			delete e;
			e = nullptr;
			if (!recur)
				return;
		}
	}
}

void cpio::makedir(mode_t mode, const char *name) {
	auto e = new cpio_entry();
	e->mode = S_IFDIR | mode;
	e->filename = strdup(name);
	insert(e);
	fprintf(stderr, "Create directory [%s] (%04o)\n", name, mode);
}

void cpio::ln(const char *target, const char *name) {
	auto e = new cpio_entry();
	e->mode = S_IFLNK;
	e->filename = strdup(name);
	e->filesize = strlen(target);
	e->data = strdup(target);
	insert(e);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", name, target);
}

void cpio::add(mode_t mode, const char *name, const char *file) {
	int fd = xopen(file, O_RDONLY);
	auto e = new cpio_entry();
	e->mode = S_IFREG | mode;
	e->filename = strdup(name);
	e->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	e->data = xmalloc(e->filesize);
	xxread(fd, e->data, e->filesize);
	close(fd);
	insert(e);
	fprintf(stderr, "Add entry [%s] (%04o)\n", name, mode);
}

bool cpio::mv(const char *from, const char *to) {
	int f = find(from), t = find(to);
	if (f >= 0) {
		if (t > 0) {
			delete arr[t];
			arr[t] = nullptr;
		}
		fprintf(stderr, "Move [%s] -> [%s]\n", from, to);
		char * tmp = strdup(to);
		free(arr[f]->filename);
		arr[f]->filename = tmp;
		return true;
	}
	fprintf(stderr, "Cannot find entry %s\n", from);
	return false;
}

void cpio::extract() {
	for (auto &e : arr) {
		if (!e)
			continue;
		fprintf(stderr, "Extract [%s]\n", e->filename);
		unlink(e->filename);
		rmdir(e->filename);
		if (S_ISDIR(e->mode)) {
			mkdir(e->filename, e->mode & 0777);
		} else if (S_ISREG(e->mode)) {
			int fd = creat(e->filename, e->mode & 0777);
			xwrite(fd, e->data, e->filesize);
			fchown(fd, e->uid, e->gid);
			close(fd);
		} else if (S_ISLNK(e->mode)) {
			char *target = new char[e->filesize + 1];
			memcpy(target, e->data, e->filesize);
			symlink(target, e->filename);
			delete[] target;
		}
	}
}

bool cpio::extract(const char *name, const char *file) {
	int i = find(name);
	if (i > 0) {
		auto e = arr[i];
		fprintf(stderr, "Extract [%s] to [%s]\n", name, file);
		if (S_ISREG(e->mode)) {
			int fd = creat(file, e->mode & 0777);
			xwrite(fd, e->data, e->filesize);
			fchown(fd, e->uid, e->gid);
			close(fd);
		} else if (S_ISLNK(e->mode)) {
			char *target = new char[e->filesize + 1];
			memcpy(target, e->data, e->filesize);
			symlink(target, e->filename);
			delete[] target;
		}
		return true;
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", name);
	return false;
}

void cpio::sort() {
	arr.sort();
	while (arr.back() == nullptr)
		arr.pop_back();
}
